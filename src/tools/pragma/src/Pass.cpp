/*
 * Copyright 2024 - Federico Sossai, Simone Campanoni
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
#include <string>

#include "arcana/noelle/core/NoellePass.hpp"
#include "arcana/noelle/core/MultiExitRegionTree.hpp"
#include "arcana/noelle/core/PragmaManager.hpp"

using namespace std;
using namespace llvm;
using namespace arcana::noelle;

namespace arcana::noelle {

class Pragma : public ModulePass {
public:
  static char ID;
  Pragma();
  ~Pragma() = default;
  bool doInitialization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnModule(Module &M) override;

private:
  string prefix;
};

Pragma::Pragma() : ModulePass{ ID }, prefix("Pragma: ") {}

bool Pragma::doInitialization(Module &M) {
  return false;
}

void Pragma::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<NoellePass>();
  return;
}

bool Pragma::runOnModule(Module &M) {
  errs() << prefix << "Start\n";

  // auto &noelle = getAnalysis<NoellePass>().getNoelle();
  auto &MainF = *M.getFunction("main");

  auto isBegin = [](const Instruction *I) -> bool {
    if (auto *CI = dyn_cast<CallInst>(I)) {
      auto callee = CI->getCalledFunction();
      if (callee) {
        if (callee->getName().startswith("noelle_pragma_begin")) {
          return true;
        }
      }
    }
    return false;
  };

  auto isEnd = [](const Instruction *I) -> bool {
    if (auto *CI = dyn_cast<CallInst>(I)) {
      auto callee = CI->getCalledFunction();
      if (callee) {
        if (callee->getName().startswith("noelle_pragma_end")) {
          return true;
        }
      }
    }
    return false;
  };

  PragmaManager PM(MainF, "test");

  auto T = PM.getPragmaTree();
  T->print(errs());

  auto T2 = T->findInnermostRegionFor(T->getChildren()[0]->getBegin());
  errs() << PM.getPragmaTreeName(T2) << "\n";

  PM.print(errs(), prefix, /*printArguments=*/true);

  return false;

  MultiExitRegionTree MERT(MainF, isBegin, isEnd);

  MERT.print(errs(), prefix);

  auto R1 = MERT.getChildren()[0];
  auto R2 = R1->getChildren()[0];
  auto R3 = R2->getChildren()[0];
  auto R4 = R2->getChildren()[1];
  auto R5 = MERT.getChildren()[1];
  auto R6 = R5->getChildren()[0];

  errs() << "R0 = " << &MERT << "\n";
  errs() << "R1 = " << R1 << "\n";
  errs() << "R2 = " << R2 << "\n";
  errs() << "R3 = " << R3 << "\n";
  errs() << "R4 = " << R4 << "\n";
  errs() << "R5 = " << R5 << "\n";
  errs() << "R6 = " << R6 << "\n";

  errs() << "outermost R0 R1 " << MERT.findOutermostRegionFor(R1->getBegin())
         << "\n";
  errs() << "outermost R0 R2 " << MERT.findOutermostRegionFor(R2->getBegin())
         << "\n";
  errs() << "outermost R0 R3 " << MERT.findOutermostRegionFor(R3->getBegin())
         << "\n";
  errs() << "outermost R0 R4 " << MERT.findOutermostRegionFor(R4->getBegin())
         << "\n";
  errs() << "outermost R0 R5 " << MERT.findOutermostRegionFor(R5->getBegin())
         << "\n";
  errs() << "outermost R0 R6 " << MERT.findOutermostRegionFor(R6->getBegin())
         << "\n";
  errs() << "outermost R1 R1 " << R1->findOutermostRegionFor(R1->getBegin())
         << "\n";
  errs() << "outermost R1 R2 " << R1->findOutermostRegionFor(R2->getBegin())
         << "\n";
  errs() << "outermost R1 R3 " << R1->findOutermostRegionFor(R3->getBegin())
         << "\n";
  errs() << "outermost R1 R4 " << R1->findOutermostRegionFor(R4->getBegin())
         << "\n";
  errs() << "outermost R2 R3 " << R2->findOutermostRegionFor(R3->getBegin())
         << "\n";
  errs() << "outermost R3 R4 " << R3->findOutermostRegionFor(R4->getBegin())
         << "\n";
  errs() << "outermost R4 R3 " << R4->findOutermostRegionFor(R3->getBegin())
         << "\n";
  errs() << "\n";
  errs() << "innermost R0 R1 " << MERT.findInnermostRegionFor(R1->getBegin())
         << "\n";
  errs() << "innermost R0 R2 " << MERT.findInnermostRegionFor(R2->getBegin())
         << "\n";
  errs() << "innermost R0 R3 " << MERT.findInnermostRegionFor(R3->getBegin())
         << "\n";
  errs() << "innermost R0 R4 " << MERT.findInnermostRegionFor(R4->getBegin())
         << "\n";
  errs() << "innermost R0 R5 " << MERT.findInnermostRegionFor(R5->getBegin())
         << "\n";
  errs() << "innermost R0 R6 " << MERT.findInnermostRegionFor(R6->getBegin())
         << "\n";
  errs() << "innermost R2 R2 " << R2->findInnermostRegionFor(R2->getBegin())
         << "\n";
  errs() << "innermost R3 R3 " << R3->findInnermostRegionFor(R3->getBegin())
         << "\n";
  errs() << "innermost R3 R4 " << R3->findInnermostRegionFor(R4->getBegin())
         << "\n";
  errs() << "innermost R4 R3 " << R4->findInnermostRegionFor(R3->getBegin())
         << "\n";
  errs() << "innermost R4 R4 " << R4->findInnermostRegionFor(R4->getBegin())
         << "\n";
  errs() << "innermost R5 R1 " << R5->findInnermostRegionFor(R1->getBegin())
         << "\n";
  errs() << "innermost R1 R5 " << R1->findInnermostRegionFor(R5->getBegin())
         << "\n";

  auto showPath = [](const auto &path) {
    for (auto T : path) {
      errs() << *T->getBegin() << "\n";
    }
  };

  errs() << "path to R1 ";
  showPath(MERT.getPathTo(R1->getBegin()));
  errs() << "path to R2 ";
  showPath(MERT.getPathTo(R2->getBegin()));
  errs() << "path to R3 ";
  showPath(MERT.getPathTo(R3->getBegin()));
  errs() << "path to R4 ";
  showPath(MERT.getPathTo(R4->getBegin()));
  errs() << "path to R5 ";
  showPath(MERT.getPathTo(R5->getBegin()));
  errs() << "path to R6 ";
  showPath(MERT.getPathTo(R6->getBegin()));

  errs() << "path R2 to R2 ";
  showPath(R2->getPathTo(R2->getBegin()));
  errs() << "path R2 to R3 ";
  showPath(R2->getPathTo(R3->getBegin()));
  errs() << "path R2 to R5 ";
  showPath(R2->getPathTo(R5->getBegin()));
  errs() << "path R3 to R2 ";
  showPath(R3->getPathTo(R2->getBegin()));

  errs() << "\n";

  // PragmaManager PM(MainF, "hello");

  errs() << prefix << "End\n";
  return false;
}

} // namespace arcana::noelle

char Pragma::ID = 0;
static RegisterPass<Pragma> X("Pragma", "");
