//===-- HelloWorld.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/HelloNew/HelloWorld.h"

using namespace llvm;

PreservedAnalyses HelloWorldPass::run(Function &F,
                                      FunctionAnalysisManager &AM) {
  outs() << "===============================================\n";
  outs() << "Function: " << F.getName() << "\n";
  outs() << "Function instruction count: " << F.getInstructionCount() << "\n";

  // https://llvm.org/docs/ProgrammersManual.html#iterating-over-the-basicblock-in-a-function
  for (const auto &bb : F) {
    errs() << "Basic block (name=" << bb.getName() << ") has "
             << bb.size() << " instructions.\n";
    outs() << "X BB: " << bb.getName() << "\n";
    outs() << "X BB Parent: " << bb.getParent() << "\n";
  }

  return PreservedAnalyses::all();
}
