/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "IVAttrTestSuite.hpp"

using namespace llvm;

// Register pass to "opt"
char IVAttrTestSuite::ID = 0;
static RegisterPass<IVAttrTestSuite> X("UnitTester", "IV Attribute Unit Tester");

// Register pass to "clang"
static IVAttrTestSuite * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new IVAttrTestSuite());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new IVAttrTestSuite());}});// ** for -O0

const char *IVAttrTestSuite::tests[] = {
  "verifyStartAndStepByLoop",
  "verifyIntermediateValues",
  "verifyLoopGoverning"
};
TestFunction IVAttrTestSuite::testFns[] = {
  IVAttrTestSuite::verifyStartAndStepByLoop,
  IVAttrTestSuite::verifyIntermediateValues,
  IVAttrTestSuite::verifyLoopGoverning
};

bool IVAttrTestSuite::doInitialization (Module &M) {
  errs() << "IVAttrTestSuite: Initialize\n";
  const int numTests = sizeof(tests) / sizeof(tests[0]);
  this->suite = new TestSuite("IVAttrTestSuite", tests, testFns, numTests, "test.txt");
  this->M = &M;
  return false;
}

void IVAttrTestSuite::getAnalysisUsage (AnalysisUsage &AU) const {
  AU.addRequired<PDGAnalysis>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<CallGraphWrapperPass>();
}

bool IVAttrTestSuite::runOnModule (Module &M) {
  errs() << "IVAttrTestSuite: Start\n";
  auto mainFunction = M.getFunction("main");

  // this->LI = &getAnalysis<LoopInfoWrapperPass>(*mainFunction).getLoopInfo();
  // // TODO: Grab first loop and produce attributes on it
  // LoopsSummary LIS;
  // Loop *topLoop = LI->getLoopsInPreorder()[0];
  // LIS.populate(*LI, topLoop);

  // auto *DT = &getAnalysis<DominatorTreeWrapperPass>(*mainFunction).getDomTree();
  // auto *PDT = &getAnalysis<PostDominatorTreeWrapperPass>(*mainFunction).getPostDomTree();
  // DominatorSummary DS(*DT, *PDT);

  // this->fdg = getAnalysis<PDGAnalysis>().getFunctionPDG(*mainFunction);
  // this->sccdag = new SCCDAG(fdg);

  // this->SE = &getAnalysis<ScalarEvolutionWrapperPass>(*mainFunction).getSE();

  // this->attrs = new IVAttrs();
  // this->attrs->populate(sccdag, LIS, *SE, DS);

  // auto loopDG = fdg->createLoopsSubgraph(topLoop);
  // this->sccdagTopLoopNorm = new SCCDAG(loopDG);
  // // PDGPrinter printer;
  // // printer.writeGraph<SCCDAG>("graph-top-loop.dot", sccdagTopLoopNorm);
  // SCCDAGNormalizer normalizer(*sccdagTopLoopNorm, LIS, *SE, DS);
  // normalizer.normalizeInPlace();

  // suite->runTests((ModulePass &)*this);

  // delete this->attrs;
  // delete this->sccdag;
  // delete this->sccdagTopLoopNorm;
  // delete fdg;

  return false;
}