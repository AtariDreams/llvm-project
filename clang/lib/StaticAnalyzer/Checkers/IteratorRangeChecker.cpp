//===-- IteratorRangeChecker.cpp ----------------------------------*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Defines a checker for dereference of the past-the-end iterator and
// out-of-range increments and decrements.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"


#include "Iterator.h"

using namespace clang;
using namespace ento;
using namespace iterator;

namespace {

class IteratorRangeChecker
  : public Checker<check::PreCall> {

  std::unique_ptr<BugType> OutOfRangeBugType;

  void verifyDereference(CheckerContext &C, SVal Val) const;
  void verifyIncrement(CheckerContext &C, SVal Iter) const;
  void verifyDecrement(CheckerContext &C, SVal Iter) const;
  void verifyRandomIncrOrDecr(CheckerContext &C, OverloadedOperatorKind Op,
                              SVal LHS, SVal RHS) const;
  void verifyAdvance(CheckerContext &C, SVal LHS, SVal RHS) const;
  void verifyPrev(CheckerContext &C, SVal LHS, SVal RHS) const;
  void verifyNext(CheckerContext &C, SVal LHS, SVal RHS) const;
  void reportBug(const StringRef &Message, SVal Val, CheckerContext &C,
                 ExplodedNode *ErrNode) const;

public:
  IteratorRangeChecker();

  void checkPreCall(const CallEvent &Call, CheckerContext &C) const;

  using AdvanceFn = void (IteratorRangeChecker::*)(CheckerContext &, SVal,
                                                   SVal) const;

  CallDescriptionMap<AdvanceFn> AdvanceFunctions = {
      {{{"std", "advance"}, 2}, &IteratorRangeChecker::verifyAdvance},
      {{{"std", "prev"}, 2}, &IteratorRangeChecker::verifyPrev},
      {{{"std", "next"}, 2}, &IteratorRangeChecker::verifyNext},
  };
};

bool isPastTheEnd(ProgramStateRef State, const IteratorPosition &Pos);
bool isAheadOfRange(ProgramStateRef State, const IteratorPosition &Pos);
bool isBehindPastTheEnd(ProgramStateRef State, const IteratorPosition &Pos);
bool isZero(ProgramStateRef State, const NonLoc &Val);

} //namespace

IteratorRangeChecker::IteratorRangeChecker() {
  OutOfRangeBugType.reset(
      new BugType(this, "Iterator out of range", "Misuse of STL APIs"));
}

void IteratorRangeChecker::checkPreCall(const CallEvent &Call,
                                        CheckerContext &C) const {
  // Check for out of range access
  const auto *Func = dyn_cast_or_null<FunctionDecl>(Call.getDecl());
  if (!Func)
    return;

  if (Func->isOverloadedOperator()) {
    if (isIncrementOperator(Func->getOverloadedOperator())) {
      // Check for out-of-range incrementions
      if (const auto *InstCall = dyn_cast<CXXInstanceCall>(&Call)) {
        verifyIncrement(C, InstCall->getCXXThisVal());
      } else {
        if (Call.getNumArgs() >= 1) {
          verifyIncrement(C, Call.getArgSVal(0));
        }
      }
    } else if (isDecrementOperator(Func->getOverloadedOperator())) {
      // Check for out-of-range decrementions
      if (const auto *InstCall = dyn_cast<CXXInstanceCall>(&Call)) {
        verifyDecrement(C, InstCall->getCXXThisVal());
      } else {
        if (Call.getNumArgs() >= 1) {
          verifyDecrement(C, Call.getArgSVal(0));
        }
      }
    } else if (isRandomIncrOrDecrOperator(Func->getOverloadedOperator())) {
      if (const auto *InstCall = dyn_cast<CXXInstanceCall>(&Call)) {
        // Check for out-of-range incrementions and decrementions
        if (Call.getNumArgs() >= 1 &&
            Call.getArgExpr(0)->getType()->isIntegralOrEnumerationType()) {
          verifyRandomIncrOrDecr(C, Func->getOverloadedOperator(),
                                 InstCall->getCXXThisVal(),
                                 Call.getArgSVal(0));
        }
      } else {
        if (Call.getNumArgs() >= 2 &&
            Call.getArgExpr(1)->getType()->isIntegralOrEnumerationType()) {
          verifyRandomIncrOrDecr(C, Func->getOverloadedOperator(),
                                 Call.getArgSVal(0), Call.getArgSVal(1));
        }
      }
    } else if (isDereferenceOperator(Func->getOverloadedOperator())) {
      // Check for dereference of out-of-range iterators
      if (const auto *InstCall = dyn_cast<CXXInstanceCall>(&Call)) {
        verifyDereference(C, InstCall->getCXXThisVal());
      } else {
        verifyDereference(C, Call.getArgSVal(0));
      }
    }
  } else {
    const AdvanceFn *Verifier = AdvanceFunctions.lookup(Call);
    if (Verifier) {
      if (Call.getNumArgs() > 1) {
        (this->**Verifier)(C, Call.getArgSVal(0), Call.getArgSVal(1));
      } else {
        auto &BVF = C.getSValBuilder().getBasicValueFactory();
        (this->**Verifier)(
            C, Call.getArgSVal(0),
            nonloc::ConcreteInt(BVF.getValue(llvm::APSInt::get(1))));
      }
    }
  }
}

void IteratorRangeChecker::verifyDereference(CheckerContext &C,
                                             SVal Val) const {
  auto State = C.getState();
  const auto *Pos = getIteratorPosition(State, Val);
  if (Pos && isPastTheEnd(State, *Pos)) {
    auto *N = C.generateErrorNode(State);
    if (!N)
      return;
    reportBug("Past-the-end iterator dereferenced.", Val, C, N);
    return;
  }
}

void IteratorRangeChecker::verifyIncrement(CheckerContext &C, SVal Iter) const {
  auto &BVF = C.getSValBuilder().getBasicValueFactory();
  verifyRandomIncrOrDecr(C, OO_Plus, Iter,
                     nonloc::ConcreteInt(BVF.getValue(llvm::APSInt::get(1))));
}

void IteratorRangeChecker::verifyDecrement(CheckerContext &C, SVal Iter) const {
  auto &BVF = C.getSValBuilder().getBasicValueFactory();
  verifyRandomIncrOrDecr(C, OO_Minus, Iter,
                     nonloc::ConcreteInt(BVF.getValue(llvm::APSInt::get(1))));
}

void IteratorRangeChecker::verifyRandomIncrOrDecr(CheckerContext &C,
                                                  OverloadedOperatorKind Op,
                                                  SVal LHS, SVal RHS) const {
  auto State = C.getState();

  auto Value = RHS;
  if (auto ValAsLoc = RHS.getAs<Loc>()) {
    Value = State->getRawSVal(*ValAsLoc);
  }

  if (Value.isUnknown())
    return;

  // Incremention or decremention by 0 is never a bug.
  if (isZero(State, Value.castAs<NonLoc>()))
    return;

  // The result may be the past-end iterator of the container, but any other
  // out of range position is undefined behaviour
  auto StateAfter = advancePosition(State, LHS, Op, Value);
  if (!StateAfter)
    return;

  const auto *PosAfter = getIteratorPosition(StateAfter, LHS);
  assert(PosAfter &&
         "Iterator should have position after successful advancement");
  if (isAheadOfRange(State, *PosAfter)) {
    auto *N = C.generateErrorNode(State);
    if (!N)
      return;
    reportBug("Iterator decremented ahead of its valid range.", LHS,
                        C, N);
  }
  if (isBehindPastTheEnd(State, *PosAfter)) {
    auto *N = C.generateErrorNode(State);
    if (!N)
      return;
    reportBug("Iterator incremented behind the past-the-end "
                        "iterator.", LHS, C, N);
  }
}

void IteratorRangeChecker::verifyAdvance(CheckerContext &C, SVal LHS,
                                         SVal RHS) const {
  verifyRandomIncrOrDecr(C, OO_PlusEqual, LHS, RHS);
}

void IteratorRangeChecker::verifyPrev(CheckerContext &C, SVal LHS,
                                      SVal RHS) const {
  verifyRandomIncrOrDecr(C, OO_Minus, LHS, RHS);
}

void IteratorRangeChecker::verifyNext(CheckerContext &C, SVal LHS,
                                      SVal RHS) const {
  verifyRandomIncrOrDecr(C, OO_Plus, LHS, RHS);
}

void IteratorRangeChecker::reportBug(const StringRef &Message, SVal Val,
                                     CheckerContext &C,
                                     ExplodedNode *ErrNode) const {
  auto R = std::make_unique<PathSensitiveBugReport>(*OutOfRangeBugType, Message,
                                                    ErrNode);

  const auto *Pos = getIteratorPosition(C.getState(), Val);
  assert(Pos && "Iterator without known position cannot be out-of-range.");

  R->markInteresting(Val);
  R->markInteresting(Pos->getContainer());
  C.emitReport(std::move(R));
}

namespace {

bool isLess(ProgramStateRef State, SymbolRef Sym1, SymbolRef Sym2);
bool isGreater(ProgramStateRef State, SymbolRef Sym1, SymbolRef Sym2);
bool isEqual(ProgramStateRef State, SymbolRef Sym1, SymbolRef Sym2);

bool isZero(ProgramStateRef State, const NonLoc &Val) {
  auto &BVF = State->getBasicVals();
  return compare(State, Val,
                 nonloc::ConcreteInt(BVF.getValue(llvm::APSInt::get(0))),
                 BO_EQ);
}

bool isPastTheEnd(ProgramStateRef State, const IteratorPosition &Pos) {
  const auto *Cont = Pos.getContainer();
  const auto *CData = getContainerData(State, Cont);
  if (!CData)
    return false;

  const auto End = CData->getEnd();
  if (End) {
    if (isEqual(State, Pos.getOffset(), End)) {
      return true;
    }
  }

  return false;
}

bool isAheadOfRange(ProgramStateRef State, const IteratorPosition &Pos) {
  const auto *Cont = Pos.getContainer();
  const auto *CData = getContainerData(State, Cont);
  if (!CData)
    return false;

  const auto Beg = CData->getBegin();
  if (Beg) {
    if (isLess(State, Pos.getOffset(), Beg)) {
      return true;
    }
  }

  return false;
}

bool isBehindPastTheEnd(ProgramStateRef State, const IteratorPosition &Pos) {
  const auto *Cont = Pos.getContainer();
  const auto *CData = getContainerData(State, Cont);
  if (!CData)
    return false;

  const auto End = CData->getEnd();
  if (End) {
    if (isGreater(State, Pos.getOffset(), End)) {
      return true;
    }
  }

  return false;
}

bool isLess(ProgramStateRef State, SymbolRef Sym1, SymbolRef Sym2) {
  return compare(State, Sym1, Sym2, BO_LT);
}

bool isGreater(ProgramStateRef State, SymbolRef Sym1, SymbolRef Sym2) {
  return compare(State, Sym1, Sym2, BO_GT);
}

bool isEqual(ProgramStateRef State, SymbolRef Sym1, SymbolRef Sym2) {
  return compare(State, Sym1, Sym2, BO_EQ);
}

} // namespace

void ento::registerIteratorRangeChecker(CheckerManager &mgr) {
  mgr.registerChecker<IteratorRangeChecker>();
}

bool ento::shouldRegisterIteratorRangeChecker(const LangOptions &LO) {
  return true;
}
