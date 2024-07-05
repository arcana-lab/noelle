/*
 * Copyright 2021 - 2024  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "arcana/noelle/core/NoellePass.hpp"
#include "arcana/noelle/tools/LoopSize.hpp"

namespace arcana::noelle {

PreservedAnalyses LoopSize::run(Module &M, llvm::ModuleAnalysisManager &AM) {

  /*
   * Fetch NOELLE
   */
  auto &noelle = AM.getResult<NoellePass>(M);

  /*
   * Fetch the forest of loops of the program being compiled.
   */
  auto forest = noelle.getLoopNestingForest();

  /*
   * Compute the code size.
   */
  uint64_t s = 0;
  for (auto &F : M) {
    for (auto &bb : F) {
      for (auto &I : bb) {

        /*
         * Check the current instruction
         */
        if (isa<PHINode>(&I)) {
          continue;
        }
        if (auto brInst = dyn_cast<BranchInst>(&I)) {
          if (brInst->isUnconditional()) {
            continue;
          }
        }

        /*
         * Fetch the innermost loop that contains the instruction.
         */
        auto loopNode = forest->getInnermostLoopThatContains(&I);
        if (loopNode == nullptr) {

          /*
           * The instruction isn't in any loop
           */
          continue;
        }
        auto loop = loopNode->getLoop();
        assert(loop != nullptr);
        assert(loop->isIncluded(&I));
        assert(!loopNode->isIncludedInItsSubLoops(&I));

        /*
         * Fetch the nesting level.
         */
        auto nl = loop->getNestingLevel();
        assert(nl >= 1);

        /*
         * Compute the instruction cost.
         */
        auto iCost = nl;

        /*
         * Update the total cost
         */
        s += iCost;
      }
    }
  }
  outs() << s << "\n";

  return PreservedAnalyses::all();
}

// Next there is code to register your pass to "opt"
llvm::PassPluginLibraryInfo getPluginInfo() {
  return { LLVM_PLUGIN_API_VERSION,
           "LoopSize",
           LLVM_VERSION_STRING,
           [](PassBuilder &PB) {
             /*
              * REGISTRATION FOR "opt -passes='LoopSize'"
              *
              */
             PB.registerPipelineParsingCallback(
                 [](StringRef Name,
                    llvm::ModulePassManager &PM,
                    ArrayRef<llvm::PassBuilder::PipelineElement>) {
                   if (Name == "LoopSize") {
                     PM.addPass(LoopSize());
                     return true;
                   }
                   return false;
                 });

             /*
              * REGISTRATION FOR "AM.getResult<NoellePass>()"
              */
             PB.registerAnalysisRegistrationCallback(
                 [](ModuleAnalysisManager &AM) {
                   AM.registerPass([&] { return NoellePass(); });
                 });
           } };
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getPluginInfo();
}

} // namespace arcana::noelle
