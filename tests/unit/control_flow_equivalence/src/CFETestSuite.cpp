/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "CFETestSuite.hpp"
#include "noelle/core/Noelle.hpp"

namespace llvm::noelle {

// Register pass to "opt"
char CFETestSuite::ID = 0;
static RegisterPass<CFETestSuite> X("UnitTester", "Control Flow Equivalence Unit Tester");

// Register pass to "clang"
static CFETestSuite * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CFETestSuite());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new CFETestSuite());}});// ** for -O0

const char *CFETestSuite::tests[] = {
  "control flow equivalent sets"
};
TestFunction CFETestSuite::testFns[] = {
  CFETestSuite::hasCorrectCFESets
};

bool CFETestSuite::doInitialization (Module &M) {
  errs() << "CFETestSuite: Initialize\n";
  const int numTests = sizeof(tests) / sizeof(tests[0]);
  this->suite = new TestSuite("CFETestSuite", tests, testFns, numTests, "test.txt");
  this->M = &M;
  return false;
}

void CFETestSuite::getAnalysisUsage (AnalysisUsage &AU) const {
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<Noelle>();
}

bool CFETestSuite::runOnModule (Module &M) {
  errs() << "CFETestSuite: Start\n";
  auto& noelle = getAnalysis<Noelle>();
  auto mainFunction = M.getFunction("main");

  /*
   * Fetch the LLVM loop
   */
  auto &LI = getAnalysis<LoopInfoWrapperPass>(*mainFunction).getLoopInfo();
  auto loop = LI.getLoopsInPreorder()[0];

  /*
   * Fetch the forest node of the loop
   */
  auto allLoopsOfFunction = noelle.getLoopStructures(mainFunction, 0);
  auto forest = noelle.organizeLoopsInTheirNestingForest(*allLoopsOfFunction);
  auto loopNode = forest->getInnermostLoopThatContains(&*loop->getHeader()->begin());

  /*
   * Fetch the dominators
   */
  auto DS = noelle.getDominators(mainFunction);

  /*
  std::queue<BasicBlock *> bbQ;
  for (auto bb : DT.getRoots()) bbQ.push(bb);
  while (!bbQ.empty()) {
    // Traverse DT and PDT to figure out why top level blocks aren't grouped together in CFE
    auto bb = bbQ.front();
    bbQ.pop();

    bb->printAsOperand(errs() << "NODE: "); errs() << "\t";
    auto node = DT.getNode(bb);
    for (auto child : node->getChildren()) {
      child->getBlock()->printAsOperand(errs() << " C: ");
      bbQ.push(child->getBlock());
    }
    errs() << "\n";
  }

  for (auto bb : PDT.getRoots()) bbQ.push(bb);
  while (!bbQ.empty()) {
    // Traverse DT and PDT to figure out why top level blocks aren't grouped together in CFE
    auto bb = bbQ.front();
    bbQ.pop();

    bb->printAsOperand(errs() << "PNODE: "); errs() << "\t";
    auto node = PDT.getNode(bb);
    for (auto child : node->getChildren()) {
      child->getBlock()->printAsOperand(errs() << " C: ");
      bbQ.push(child->getBlock());
    }
    errs() << "\n";
  }
  */

  this->CFE = new ControlFlowEquivalence(DS, loopNode, *mainFunction);

  suite->runTests((ModulePass &)*this);

  delete CFE;

  return false;
}

Values CFETestSuite::hasCorrectCFESets (ModulePass &pass, TestSuite &suite) {
  auto &cfePass = static_cast<CFETestSuite &>(pass);
  auto mainFunction = cfePass.M->getFunction("main");
  Values eqSets;
  for (auto &B : *mainFunction) {
    auto eqSet = cfePass.CFE->getEquivalences(&B);
    std::string eqSetStr;
    for (auto eqB : eqSet) {
      eqSetStr += suite.printAsOperandToString(eqB) += suite.unorderedValueDelimiter;
    }
    eqSets.insert(eqSetStr.substr(0, eqSetStr.length() - 1));
  }
  return eqSets;
}

}
