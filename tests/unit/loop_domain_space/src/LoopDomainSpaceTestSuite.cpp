/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *

 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopDomainSpaceTestSuite.hpp"

using namespace llvm;

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
  "verifyDisjointAccessBetweenIterations"
};

TestFunction LoopDomainSpaceTestSuite::testFns[] = {
  LoopDomainSpaceTestSuite::verifyDisjointAccessBetweenIterations
};

bool LoopDomainSpaceTestSuite::doInitialization (Module &M) {
  errs() << "LoopDomainSpaceTestSuite: Initialize\n";
  const int numTests = sizeof(tests) / sizeof(tests[0]);
  this->suite = new TestSuite("LoopDomainSpaceTestSuite", tests, testFns, numTests, "test.txt");
  this->M = &M;
  return false;
}

void LoopDomainSpaceTestSuite::getAnalysisUsage (AnalysisUsage &AU) const {
  AU.addRequired<PDGAnalysis>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
}

bool LoopDomainSpaceTestSuite::runOnModule (Module &M) {
  errs() << "LoopDomainSpaceTestSuite: Start\n";

  auto mainFunction = M.getFunction("main");
  auto LI = &getAnalysis<LoopInfoWrapperPass>(*mainFunction).getLoopInfo();
  auto SE = &getAnalysis<ScalarEvolutionWrapperPass>(*mainFunction).getSE();
  auto fdg = getAnalysis<PDGAnalysis>().getFunctionPDG(*mainFunction);
  Loop *topLoop = LI->getLoopsInPreorder()[0];
  auto loopDG = fdg->createLoopsSubgraph(topLoop);
  auto loopSCCDAG = new SCCDAG(loopDG);

  this->LIS = new LoopsSummary(topLoop);
  auto loopExitBlocks = LIS->getLoopNestingTreeRoot()->getLoopExitBasicBlocks();
  auto environment = new LoopEnvironment(loopDG, loopExitBlocks);
  InvariantManager invariantManager(LIS->getLoopNestingTreeRoot(), loopDG);
  auto IVs = new InductionVariableManager(*LIS, invariantManager, *SE, *loopSCCDAG, *environment);
  this->domainSpaceAnalysis = new LoopIterationDomainSpaceAnalysis(*LIS, *IVs, *SE);

  suite->runTests((ModulePass &)*this);

  delete this->domainSpaceAnalysis;
  delete IVs;
  delete this->LIS;
  delete environment;
  delete loopSCCDAG;
  delete loopDG;
  delete fdg;

  return false;
}

Values LoopDomainSpaceTestSuite::verifyDisjointAccessBetweenIterations (ModulePass &pass, TestSuite &suite) {
  LoopDomainSpaceTestSuite &attrPass = static_cast<LoopDomainSpaceTestSuite &>(pass);

  Values disjointBetweenIterations;
  std::unordered_set<Instruction *> memoryAccesses;
  for (auto B : attrPass.LIS->getLoopNestingTreeRoot()->getBasicBlocks()) {
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
