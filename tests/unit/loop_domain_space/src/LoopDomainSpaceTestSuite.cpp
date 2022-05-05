/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *

 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopDomainSpaceTestSuite.hpp"

using namespace parallelizertests;

namespace llvm::noelle {

// Register pass to "opt"
char LoopDomainSpaceTestSuite::ID = 0;
static RegisterPass<LoopDomainSpaceTestSuite> X("UnitTester", "Loop Domain Space Unit Tester");

// Register pass to "clang"
static LoopDomainSpaceTestSuite * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new LoopDomainSpaceTestSuite());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new LoopDomainSpaceTestSuite());}});// ** for -O0

const char *LoopDomainSpaceTestSuite::tests[] = {
  "verifyDisjointAccessBetweenIterations",
  "verifyDisjointAccessBetweenIterationsAfterSCEVSimplification"
};

TestFunction LoopDomainSpaceTestSuite::testFns[] = {
  LoopDomainSpaceTestSuite::verifyDisjointAccessBetweenIterations,
  LoopDomainSpaceTestSuite::verifyDisjointAccessBetweenIterationsAfterSCEVSimplification
};

bool LoopDomainSpaceTestSuite::doInitialization (Module &M) {
  errs() << "LoopDomainSpaceTestSuite: Initialize\n";
  const int numTests = sizeof(tests) / sizeof(tests[0]);
  this->suite = new TestSuite("LoopDomainSpaceTestSuite", tests, testFns, numTests, "test.txt");
  this->M = &M;
  return false;
}

void LoopDomainSpaceTestSuite::getAnalysisUsage (AnalysisUsage &AU) const {
  AU.addRequired<Noelle>();
  AU.addRequired<PDGAnalysis>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
}

bool LoopDomainSpaceTestSuite::runOnModule (Module &M) {
  errs() << "LoopDomainSpaceTestSuite: Start\n";

  suite->runTests((ModulePass &)*this);

  if (this->loopNode) { delete this->loopNode; this->loopNode = nullptr; }
  if (this->IVM) { delete this->IVM; this->IVM = nullptr; }
  if (this->domainSpaceAnalysis) { delete this->domainSpaceAnalysis; this->domainSpaceAnalysis = nullptr; }

  return false;
}

Values LoopDomainSpaceTestSuite::verifyDisjointAccessBetweenIterations (ModulePass &pass, TestSuite &suite) {
  LoopDomainSpaceTestSuite &attrPass = static_cast<LoopDomainSpaceTestSuite &>(pass);
  attrPass.computeAnalysisWithoutSCEVSimplification ();
  return attrPass.collectDisjointAccessesBetweenIterations(pass, suite);
}

Values LoopDomainSpaceTestSuite::verifyDisjointAccessBetweenIterationsAfterSCEVSimplification (
  ModulePass &pass,
  TestSuite &suite
) {
  LoopDomainSpaceTestSuite &attrPass = static_cast<LoopDomainSpaceTestSuite &>(pass);
  attrPass.computeAnalysisWithSCEVSimplification ();
  return attrPass.collectDisjointAccessesBetweenIterations(pass, suite);
}

Values LoopDomainSpaceTestSuite::collectDisjointAccessesBetweenIterations (ModulePass &pass, TestSuite &suite) {
  LoopDomainSpaceTestSuite &attrPass = static_cast<LoopDomainSpaceTestSuite &>(pass);

  Values disjointBetweenIterations;
  std::unordered_set<Instruction *> memoryAccesses;
  for (auto B : attrPass.loopNode->getLoop()->getBasicBlocks()) {
    for (auto &I : *B) {
      if (isa<StoreInst>(&I) || isa<LoadInst>(&I)) {
        memoryAccesses.insert(&I);
      }
    }
  }

  for (auto access1 : memoryAccesses) {
    for (auto access2 : memoryAccesses) {
      // TODO: Should this be ignored for store -> store ?
      if (access1 == access2) continue;

      if (!attrPass.domainSpaceAnalysis->
        areInstructionsAccessingDisjointMemoryLocationsBetweenIterations(access1, access2)) continue;

      auto unorderedValueString = suite.combineUnorderedValues(
        std::vector<std::string>{ suite.printToString(access1), suite.printToString(access2) }
      );
      disjointBetweenIterations.insert(unorderedValueString);
    }
  }

  return disjointBetweenIterations;
}

void LoopDomainSpaceTestSuite::computeAnalysisWithoutSCEVSimplification (void) {
  assert(!modifiedCodeWithSCEVSimplification && "Can't compute non-simplified analysis after simplifying!");

  if (this->loopNode) { delete this->loopNode; this->loopNode = nullptr; }
  if (this->IVM) { delete this->IVM; this->IVM = nullptr; }
  if (this->domainSpaceAnalysis) { delete this->domainSpaceAnalysis; this->domainSpaceAnalysis = nullptr; }

  auto mainFunction = M->getFunction("main");
  auto LI = &getAnalysis<LoopInfoWrapperPass>(*mainFunction).getLoopInfo();
  auto SE = &getAnalysis<ScalarEvolutionWrapperPass>(*mainFunction).getSE();
  getAnalysis<PDGAnalysis>().releaseMemory();
  auto fdg = getAnalysis<PDGAnalysis>().getFunctionPDG(*mainFunction);
  auto topLoop = LI->getLoopsInPreorder()[0];
  auto loopDG = fdg->createLoopsSubgraph(topLoop);
  SCCDAG loopSCCDAG(loopDG);

  /*
   * Fetch the forest node of the loop
   */
  errs() << "Constructing Loops summary\n";
  auto& noelle = getAnalysis<Noelle>();
  auto allLoopsOfFunction = noelle.getLoopStructures(mainFunction, 0);
  auto forest = noelle.organizeLoopsInTheirNestingForest(*allLoopsOfFunction);
  this->loopNode = forest->getInnermostLoopThatContains(&*topLoop->getHeader()->begin());

  auto loopExitBlocks = loopNode->getLoop()->getLoopExitBasicBlocks();
  errs() << "Constructing environment\n";
  LoopEnvironment environment(loopDG, loopExitBlocks);
  errs() << "Constructing invariant manager\n";
  InvariantManager invariantManager(loopNode->getLoop(), loopDG);
  errs() << "Constructing IV manager\n";
  this->IVM = new InductionVariableManager(loopNode, invariantManager, *SE, loopSCCDAG, environment, *topLoop);
  errs() << "Constructing loop iteration domain space analysis\n";
  this->domainSpaceAnalysis = new LoopIterationDomainSpaceAnalysis(loopNode, *IVM, *SE);
  errs() << "Finished\n";
}

void LoopDomainSpaceTestSuite::computeAnalysisWithSCEVSimplification (void) {
  assert(!modifiedCodeWithSCEVSimplification && "Can only simplify once!");
  auto& noelle = getAnalysis<Noelle>();

  if (this->loopNode) { delete this->loopNode; this->loopNode = nullptr; }
  if (this->IVM) { delete this->IVM; this->IVM = nullptr; }
  if (this->domainSpaceAnalysis) { delete this->domainSpaceAnalysis; this->domainSpaceAnalysis = nullptr; }

  auto mainFunction = M->getFunction("main");
  auto LI = &getAnalysis<LoopInfoWrapperPass>(*mainFunction).getLoopInfo();
  auto SE = &getAnalysis<ScalarEvolutionWrapperPass>(*mainFunction).getSE();
  getAnalysis<PDGAnalysis>().releaseMemory();
  auto fdg = getAnalysis<PDGAnalysis>().getFunctionPDG(*mainFunction);
  Loop *topLoop = LI->getLoopsInPreorder()[0];
  auto loopDG = fdg->createLoopsSubgraph(topLoop);
  SCCDAG loopSCCDAG(loopDG);

  /*
   * Fetch the forest node of the loop
   */
  auto allLoopsOfFunction = noelle.getLoopStructures(mainFunction, 0);
  auto forest = noelle.organizeLoopsInTheirNestingForest(*allLoopsOfFunction);
  this->loopNode = forest->getInnermostLoopThatContains(&*topLoop->getHeader()->begin());

  errs() << "Constructing Loops summary\n";
  auto loopExitBlocks = loopNode->getLoop()->getLoopExitBasicBlocks();
  errs() << "Constructing environment\n";
  LoopEnvironment environment(loopDG, loopExitBlocks);
  errs() << "Constructing invariant manager\n";
  InvariantManager invariantManager(loopNode->getLoop(), loopDG);
  errs() << "Constructing IV manager\n";
  this->IVM = new InductionVariableManager(loopNode, invariantManager, *SE, loopSCCDAG, environment, *topLoop);

  SCEVSimplification scevSimplify(noelle);
  errs() << "Running SCEVSimplification\n";
  scevSimplify.simplifyIVRelatedSCEVs(loopNode, &invariantManager, IVM);

  computeAnalysisWithoutSCEVSimplification();
  errs() << "Finished with simplification\n";

  modifiedCodeWithSCEVSimplification = true;
}

}
