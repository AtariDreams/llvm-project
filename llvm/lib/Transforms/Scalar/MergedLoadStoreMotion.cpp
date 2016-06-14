//===- MergedLoadStoreMotion.cpp - merge and hoist/sink load/stores -------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//! \file
//! \brief This pass performs merges of loads and stores on both sides of a
//  diamond (hammock). It hoists the loads and sinks the stores.
//
// The algorithm iteratively hoists two loads to the same address out of a
// diamond (hammock) and merges them into a single load in the header. Similar
// it sinks and merges two stores to the tail block (footer). The algorithm
// iterates over the instructions of one side of the diamond and attempts to
// find a matching load/store on the other side. It hoists / sinks when it
// thinks it safe to do so.  This optimization helps with eg. hiding load
// latencies, triggering if-conversion, and reducing static code size.
//
//===----------------------------------------------------------------------===//
//
//
// Example:
// Diamond shaped code before merge:
//
//            header:
//                     br %cond, label %if.then, label %if.else
//                        +                    +
//                       +                      +
//                      +                        +
//            if.then:                         if.else:
//               %lt = load %addr_l               %le = load %addr_l
//               <use %lt>                        <use %le>
//               <...>                            <...>
//               store %st, %addr_s               store %se, %addr_s
//               br label %if.end                 br label %if.end
//                     +                         +
//                      +                       +
//                       +                     +
//            if.end ("footer"):
//                     <...>
//
// Diamond shaped code after merge:
//
//            header:
//                     %l = load %addr_l
//                     br %cond, label %if.then, label %if.else
//                        +                    +
//                       +                      +
//                      +                        +
//            if.then:                         if.else:
//               <use %l>                         <use %l>
//               <...>                            <...>
//               br label %if.end                 br label %if.end
//                      +                        +
//                       +                      +
//                        +                    +
//            if.end ("footer"):
//                     %s.sink = phi [%st, if.then], [%se, if.else]
//                     <...>
//                     store %s.sink, %addr_s
//                     <...>
//
//
//===----------------------- TODO -----------------------------------------===//
//
// 1) Generalize to regions other than diamonds
// 2) Be more aggressive merging memory operations
// Note that both changes require register pressure control
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Scalar/MergedLoadStoreMotion.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/Loads.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"

using namespace llvm;

#define DEBUG_TYPE "mldst-motion"

//===----------------------------------------------------------------------===//
//                         MergedLoadStoreMotion Pass
//===----------------------------------------------------------------------===//

// The mergeLoad/Store algorithms could have Size0 * Size1 complexity,
// where Size0 and Size1 are the #instructions on the two sides of
// the diamond. The constant chosen here is arbitrary. Compiler Time
// Control is enforced by the check Size0 * Size1 < MagicCompileTimeControl.
const int MagicCompileTimeControl = 250;

///
/// \brief Remove instruction from parent and update memory dependence analysis.
///
static void removeInstruction(Instruction *Inst, MemoryDependenceResults *MD) {
  // Notify the memory dependence analysis.
  if (MD) {
    MD->removeInstruction(Inst);
    if (auto *LI = dyn_cast<LoadInst>(Inst))
      MD->invalidateCachedPointerInfo(LI->getPointerOperand());
    if (Inst->getType()->isPtrOrPtrVectorTy()) {
      MD->invalidateCachedPointerInfo(Inst);
    }
  }
  Inst->eraseFromParent();
}

///
/// \brief True when BB is the head of a diamond (hammock)
///
static bool isDiamondHead(BasicBlock *BB) {
  if (!BB)
    return false;
  auto *BI = dyn_cast<BranchInst>(BB->getTerminator());
  if (!BI || !BI->isConditional())
    return false;

  BasicBlock *Succ0 = BI->getSuccessor(0);
  BasicBlock *Succ1 = BI->getSuccessor(1);

  if (!Succ0->getSinglePredecessor())
    return false;
  if (!Succ1->getSinglePredecessor())
    return false;

  BasicBlock *Succ0Succ = Succ0->getSingleSuccessor();
  BasicBlock *Succ1Succ = Succ1->getSingleSuccessor();
  // Ignore triangles.
  if (!Succ0Succ || !Succ1Succ || Succ0Succ != Succ1Succ)
    return false;
  return true;
}

///
/// \brief Return tail block of a diamond.
///
static BasicBlock *getDiamondTail(BasicBlock *BB) {
  assert(isDiamondHead(BB) && "Basic block is not head of a diamond");
  return BB->getTerminator()->getSuccessor(0)->getSingleSuccessor();
}

///
/// \brief True when instruction is a hoist barrier for a load
///
/// Whenever an instruction could possibly modify the value
/// being loaded or protect against the load from happening
/// it is considered a hoist barrier.
///
static bool isLoadHoistBarrierInRange(const Instruction &Start,
                                      const Instruction &End, LoadInst *LI,
                                      bool SafeToLoadUnconditionally,
                                      AliasAnalysis *AA) {
  if (!SafeToLoadUnconditionally)
    for (const Instruction &Inst :
         make_range(Start.getIterator(), End.getIterator()))
      if (!isGuaranteedToTransferExecutionToSuccessor(&Inst))
        return true;
  MemoryLocation Loc = MemoryLocation::get(LI);
  return AA->canInstructionRangeModRef(Start, End, Loc, MRI_Mod);
}

///
/// \brief Decide if a load can be hoisted
///
/// When there is a load in \p BB to the same address as \p LI
/// and it can be hoisted from \p BB, return that load.
/// Otherwise return Null.
///
static LoadInst *canHoistFromBlock(BasicBlock *BB1, LoadInst *Load0,
                                   AliasAnalysis *AA) {
  BasicBlock *BB0 = Load0->getParent();
  BasicBlock *Head = BB0->getSinglePredecessor();
  bool SafeToLoadUnconditionally = isSafeToLoadUnconditionally(
      Load0->getPointerOperand(), Load0->getAlignment(),
      Load0->getModule()->getDataLayout(),
      /*ScanFrom=*/Head->getTerminator());
  for (BasicBlock::iterator BBI = BB1->begin(), BBE = BB1->end(); BBI != BBE;
       ++BBI) {
    Instruction *Inst = &*BBI;

    // Only merge and hoist loads when their result in used only in BB
    auto *Load1 = dyn_cast<LoadInst>(Inst);
    if (!Load1 || Inst->isUsedOutsideOfBlock(BB1))
      continue;

    MemoryLocation Loc0 = MemoryLocation::get(Load0);
    MemoryLocation Loc1 = MemoryLocation::get(Load1);
    if (AA->isMustAlias(Loc0, Loc1) && Load0->isSameOperationAs(Load1) &&
        !isLoadHoistBarrierInRange(BB1->front(), *Load1, Load1,
                                   SafeToLoadUnconditionally, AA) &&
        !isLoadHoistBarrierInRange(BB0->front(), *Load0, Load0,
                                   SafeToLoadUnconditionally, AA)) {
      return Load1;
    }
  }
  return nullptr;
}

///
/// \brief Merge two equivalent instructions \p HoistCand and \p ElseInst into
/// \p BB
///
/// BB is the head of a diamond
///
static void hoistInstruction(BasicBlock *BB, Instruction *HoistCand,
                             Instruction *ElseInst,
                             MemoryDependenceResults *MD) {
  DEBUG(dbgs() << " Hoist Instruction into BB \n"; BB->dump();
        dbgs() << "Instruction Left\n"; HoistCand->dump(); dbgs() << "\n";
        dbgs() << "Instruction Right\n"; ElseInst->dump(); dbgs() << "\n");
  // Hoist the instruction.
  assert(HoistCand->getParent() != BB);

  // Intersect optional metadata.
  HoistCand->intersectOptionalDataWith(ElseInst);
  HoistCand->dropUnknownNonDebugMetadata();

  // Prepend point for instruction insert
  Instruction *HoistPt = BB->getTerminator();

  // Merged instruction
  Instruction *HoistedInst = HoistCand->clone();

  // Hoist instruction.
  HoistedInst->insertBefore(HoistPt);

  HoistCand->replaceAllUsesWith(HoistedInst);
  removeInstruction(HoistCand, MD);
  // Replace the else block instruction.
  ElseInst->replaceAllUsesWith(HoistedInst);
  removeInstruction(ElseInst, MD);
}

///
/// \brief Return true if no operand of \p I is defined in I's parent block
///
static bool isSafeToHoist(Instruction *I) {
  BasicBlock *Parent = I->getParent();
  for (Use &U : I->operands())
    if (auto *Instr = dyn_cast<Instruction>(&U))
      if (Instr->getParent() == Parent)
        return false;
  return true;
}

///
/// \brief Merge two equivalent loads and GEPs and hoist into diamond head
///
static bool hoistLoad(BasicBlock *BB, LoadInst *L0, LoadInst *L1,
                      MemoryDependenceResults *MD) {
  // Only one definition?
  auto *A0 = dyn_cast<Instruction>(L0->getPointerOperand());
  auto *A1 = dyn_cast<Instruction>(L1->getPointerOperand());
  if (A0 && A1 && A0->isIdenticalTo(A1) && isSafeToHoist(A0) &&
      A0->hasOneUse() && (A0->getParent() == L0->getParent()) &&
      A1->hasOneUse() && (A1->getParent() == L1->getParent()) &&
      isa<GetElementPtrInst>(A0)) {
    DEBUG(dbgs() << "Hoist Instruction into BB \n"; BB->dump();
          dbgs() << "Instruction Left\n"; L0->dump(); dbgs() << "\n";
          dbgs() << "Instruction Right\n"; L1->dump(); dbgs() << "\n");
    hoistInstruction(BB, A0, A1, MD);
    hoistInstruction(BB, L0, L1, MD);
    return true;
  }
  return false;
}

///
/// \brief Try to hoist two loads to same address into diamond header
///
/// Starting from a diamond head block, iterate over the instructions in one
/// successor block and try to match a load in the second successor.
///
static bool mergeLoads(BasicBlock *BB, AliasAnalysis *AA,
                       MemoryDependenceResults *MD) {
  bool MergedLoads = false;
  assert(isDiamondHead(BB));
  BranchInst *BI = cast<BranchInst>(BB->getTerminator());
  BasicBlock *Succ0 = BI->getSuccessor(0);
  BasicBlock *Succ1 = BI->getSuccessor(1);
  // #Instructions in Succ1 for Compile Time Control
  int Size1 = Succ1->size();
  int NLoads = 0;
  for (BasicBlock::iterator BBI = Succ0->begin(), BBE = Succ0->end();
       BBI != BBE;) {
    Instruction *I = &*BBI;
    ++BBI;

    // Don't move non-simple (atomic, volatile) loads.
    auto *L0 = dyn_cast<LoadInst>(I);
    if (!L0 || !L0->isSimple() || L0->isUsedOutsideOfBlock(Succ0))
      continue;

    ++NLoads;
    if (NLoads * Size1 >= MagicCompileTimeControl)
      break;
    if (LoadInst *L1 = canHoistFromBlock(Succ1, L0, AA)) {
      bool Res = hoistLoad(BB, L0, L1, MD);
      MergedLoads |= Res;
      // Don't attempt to hoist above loads that had not been hoisted.
      if (!Res)
        break;
    }
  }
  return MergedLoads;
}

///
/// \brief True when instruction is a sink barrier for a store
/// located in Loc
///
/// Whenever an instruction could possibly read or modify the
/// value being stored or protect against the store from
/// happening it is considered a sink barrier.
///
static bool isStoreSinkBarrierInRange(const Instruction &Start,
                                      const Instruction &End,
                                      MemoryLocation Loc, AliasAnalysis *AA) {
  for (const Instruction &Inst :
       make_range(Start.getIterator(), End.getIterator()))
    if (Inst.mayThrow())
      return true;
  return AA->canInstructionRangeModRef(Start, End, Loc, MRI_ModRef);
}

///
/// \brief Check if \p BB contains a store to the same address as \p SI
///
/// \return The store in \p  when it is safe to sink. Otherwise return Null.
///
static StoreInst *canSinkFromBlock(BasicBlock *BB1, StoreInst *Store0,
                                   AliasAnalysis *AA) {
  DEBUG(dbgs() << "can Sink? : "; Store0->dump(); dbgs() << "\n");
  BasicBlock *BB0 = Store0->getParent();
  for (BasicBlock::reverse_iterator RBI = BB1->rbegin(), RBE = BB1->rend();
       RBI != RBE; ++RBI) {
    Instruction *Inst = &*RBI;

    auto *Store1 = dyn_cast<StoreInst>(Inst);
    if (!Store1)
      continue;

    MemoryLocation Loc0 = MemoryLocation::get(Store0);
    MemoryLocation Loc1 = MemoryLocation::get(Store1);
    if (AA->isMustAlias(Loc0, Loc1) && Store0->isSameOperationAs(Store1) &&
        !isStoreSinkBarrierInRange(*Store1->getNextNode(), BB1->back(), Loc1,
                                   AA) &&
        !isStoreSinkBarrierInRange(*Store0->getNextNode(), BB0->back(), Loc0,
                                   AA)) {
      return Store1;
    }
  }
  return nullptr;
}

///
/// \brief Create a PHI node in BB for the operands of S0 and S1
///
static PHINode *getPHIOperand(BasicBlock *BB, StoreInst *S0, StoreInst *S1,
                              MemoryDependenceResults *MD) {
  // Create a phi if the values mismatch.
  Value *Opd1 = S0->getValueOperand();
  Value *Opd2 = S1->getValueOperand();
  if (Opd1 == Opd2)
    return nullptr;

  auto *NewPN = PHINode::Create(Opd1->getType(), 2, Opd2->getName() + ".sink",
                                &BB->front());
  NewPN->addIncoming(Opd1, S0->getParent());
  NewPN->addIncoming(Opd2, S1->getParent());
  if (MD && NewPN->getType()->getScalarType()->isPointerTy())
    MD->invalidateCachedPointerInfo(NewPN);
  return NewPN;
}

///
/// \brief Merge two stores to same address and sink into \p BB
///
/// Also sinks GEP instruction computing the store address
///
static bool sinkStore(BasicBlock *BB, StoreInst *S0, StoreInst *S1,
                      MemoryDependenceResults *MD) {
  // Only one definition?
  auto *A0 = dyn_cast<Instruction>(S0->getPointerOperand());
  auto *A1 = dyn_cast<Instruction>(S1->getPointerOperand());
  if (A0 && A1 && A0->isIdenticalTo(A1) && A0->hasOneUse() &&
      (A0->getParent() == S0->getParent()) && A1->hasOneUse() &&
      (A1->getParent() == S1->getParent()) && isa<GetElementPtrInst>(A0)) {
    DEBUG(dbgs() << "Sink Instruction into BB \n"; BB->dump();
          dbgs() << "Instruction Left\n"; S0->dump(); dbgs() << "\n";
          dbgs() << "Instruction Right\n"; S1->dump(); dbgs() << "\n");
    // Hoist the instruction.
    BasicBlock::iterator InsertPt = BB->getFirstInsertionPt();
    // Intersect optional metadata.
    S0->intersectOptionalDataWith(S1);
    S0->dropUnknownNonDebugMetadata();

    // Create the new store to be inserted at the join point.
    StoreInst *SNew = cast<StoreInst>(S0->clone());
    Instruction *ANew = A0->clone();
    SNew->insertBefore(&*InsertPt);
    ANew->insertBefore(SNew);

    assert(S0->getParent() == A0->getParent());
    assert(S1->getParent() == A1->getParent());

    // New PHI operand? Use it.
    if (PHINode *NewPN = getPHIOperand(BB, S0, S1, MD))
      SNew->setOperand(0, NewPN);
    removeInstruction(S0, MD);
    removeInstruction(S1, MD);
    A0->replaceAllUsesWith(ANew);
    removeInstruction(A0, MD);
    A1->replaceAllUsesWith(ANew);
    removeInstruction(A1, MD);
    return true;
  }
  return false;
}

///
/// \brief True when two stores are equivalent and can sink into the footer
///
/// Starting from a diamond tail block, iterate over the instructions in one
/// predecessor block and try to match a store in the second predecessor.
///
static bool mergeStores(BasicBlock *T, AliasAnalysis *AA,
                        MemoryDependenceResults *MD) {

  bool MergedStores = false;
  assert(T && "Footer of a diamond cannot be empty");

  pred_iterator PI = pred_begin(T), E = pred_end(T);
  assert(PI != E);
  BasicBlock *Pred0 = *PI;
  ++PI;
  BasicBlock *Pred1 = *PI;
  ++PI;
  // tail block  of a diamond/hammock?
  if (Pred0 == Pred1)
    return false; // No.
  if (PI != E)
    return false; // No. More than 2 predecessors.

  // #Instructions in Succ1 for Compile Time Control
  int Size1 = Pred1->size();
  int NStores = 0;

  for (BasicBlock::reverse_iterator RBI = Pred0->rbegin(), RBE = Pred0->rend();
       RBI != RBE;) {

    Instruction *I = &*RBI;
    ++RBI;

    // Don't sink non-simple (atomic, volatile) stores.
    auto *S0 = dyn_cast<StoreInst>(I);
    if (!S0 || !S0->isSimple())
      continue;

    ++NStores;
    if (NStores * Size1 >= MagicCompileTimeControl)
      break;
    if (StoreInst *S1 = canSinkFromBlock(Pred1, S0, AA)) {
      bool Res = sinkStore(T, S0, S1, MD);
      MergedStores |= Res;
      // Don't attempt to sink below stores that had to stick around
      // But after removal of a store and some of its feeding
      // instruction search again from the beginning since the iterator
      // is likely stale at this point.
      if (!Res)
        break;
      RBI = Pred0->rbegin();
      RBE = Pred0->rend();
      DEBUG(dbgs() << "Search again\n"; Instruction *I = &*RBI; I->dump());
    }
  }
  return MergedStores;
}

///
/// \brief Run the transformation for each function
///
static bool runMergedLoadStoreMotion(Function &F, AliasAnalysis *AA,
                                     MemoryDependenceResults *MD) {
  bool Changed = false;
  DEBUG(dbgs() << "Instruction Merger\n");

  // Merge unconditional branches, allowing PRE to catch more
  // optimization opportunities.
  for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE;) {
    BasicBlock *BB = &*FI++;

    // Hoist equivalent loads and sink stores
    // outside diamonds when possible
    if (isDiamondHead(BB)) {
      Changed |= mergeLoads(BB, AA, MD);
      Changed |= mergeStores(getDiamondTail(BB), AA, MD);
    }
  }
  return Changed;
}

PreservedAnalyses
MergedLoadStoreMotionPass::run(Function &F, AnalysisManager<Function> &AM) {
  auto &AA = AM.getResult<AAManager>(F);
  auto *MD = AM.getCachedResult<MemoryDependenceAnalysis>(F);
  if (!runMergedLoadStoreMotion(F, &AA, MD))
    return PreservedAnalyses::all();
  // FIXME: This pass should also 'preserve the CFG'.
  // The new pass manager has currently no way to do it.
  PreservedAnalyses PA;
  PA.preserve<GlobalsAA>();
  PA.preserve<MemoryDependenceAnalysis>();
  return PA;
}

namespace {
class MergedLoadStoreMotionLegacyPass : public FunctionPass {
  AliasAnalysis *AA;
  MemoryDependenceResults *MD;

public:
  static char ID; // Pass identification, replacement for typeid
  MergedLoadStoreMotionLegacyPass() : FunctionPass(ID), MD(nullptr) {
    initializeMergedLoadStoreMotionLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    AA = &getAnalysis<AAResultsWrapperPass>().getAAResults();
    auto *MDWP = getAnalysisIfAvailable<MemoryDependenceWrapperPass>();
    MD = MDWP ? &MDWP->getMemDep() : nullptr;
    return runMergedLoadStoreMotion(F, AA, MD);
  }

private:
  // This transformation requires dominator postdominator info
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<AAResultsWrapperPass>();
    AU.addPreserved<GlobalsAAWrapperPass>();
    AU.addPreserved<MemoryDependenceWrapperPass>();
  }
};

char MergedLoadStoreMotionLegacyPass::ID = 0;
} // anonymous namespace

///
/// \brief createMergedLoadStoreMotionPass - The public interface to this file.
///
FunctionPass *llvm::createMergedLoadStoreMotionPass() {
  return new MergedLoadStoreMotionLegacyPass();
}

INITIALIZE_PASS_BEGIN(MergedLoadStoreMotionLegacyPass, "mldst-motion",
                      "MergedLoadStoreMotion", false, false)
INITIALIZE_PASS_DEPENDENCY(MemoryDependenceWrapperPass)
INITIALIZE_PASS_DEPENDENCY(AAResultsWrapperPass)
INITIALIZE_PASS_END(MergedLoadStoreMotionLegacyPass, "mldst-motion",
                    "MergedLoadStoreMotion", false, false)
