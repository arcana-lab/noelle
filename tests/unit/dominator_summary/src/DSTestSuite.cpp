/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DSTestSuite.hpp"

using namespace llvm;

// Register pass to "opt"
char DSTestSuite::ID = 0;
static RegisterPass<DSTestSuite> X("UnitTester", "Dominator Summary Unit Tester");

// Register pass to "clang"
static DSTestSuite * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new DSTestSuite());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new DSTestSuite());}});// ** for -O0

const char *DSTestSuite::tests[] = {
  "dom trees are identical",
};
TestFunction DSTestSuite::testFns[] = {
  DSTestSuite::domTreesAreIdentical,
};

bool DSTestSuite::doInitialization (Module &M) {
  errs() << "DSTestSuite: Initialize\n";
  const int numTests = sizeof(tests) / sizeof(tests[0]);
  this->suite = new TestSuite("DSTestSuite", tests, testFns, numTests, "test.txt");
  this->M = &M;
  return false;
}

void DSTestSuite::getAnalysisUsage (AnalysisUsage &AU) const {
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
}

bool DSTestSuite::runOnModule (Module &M) {
  errs() << "DSTestSuite: Start\n";
  auto mainFunction = M.getFunction("main");

  this->dt = &getAnalysis<DominatorTreeWrapperPass>(*mainFunction).getDomTree();
  this->pdt = &getAnalysis<PostDominatorTreeWrapperPass>(*mainFunction).getPostDomTree();
  this->ds = new DominatorSummary(*dt, *pdt);

  suite->runTests((ModulePass &)*this);

  delete this->ds;
  return false;
}

Values DSTestSuite::domNodeIsIdentical (DSTestSuite &pass, DomTreeNodeBase<BasicBlock> &node, DomNodeSummary &nodeS) {
  BasicBlock *bb = node.getBlock();
  BasicBlock *bbS = nodeS.getBlock();
  if (bb != bbS) {
    return {
      "DomTreeNode pair aren't for the same basic block: "
        + pass.suite->printToString(bb) + " vs. "
        + pass.suite->printToString(bbS)
    };
  }
  if (node.getLevel() != nodeS.getLevel()) {
    return { "Summary on: " + pass.suite->printToString(bb) + " not at the correct depth" };
  }
  if (node.getIDom() == nullptr ^ nodeS.getIDom() == nullptr) {
    return {
      "Node does not exist across tree and summary of tree!\n"
        + std::string("Original node has immediate dominator: ") + std::to_string(node.getIDom() != nullptr)
        + "Summary node has immediate dominator: " + std::to_string(nodeS.getIDom() != nullptr)
        + pass.suite->printToString(bb)
    };
  }
  if (node.getIDom() != nullptr && node.getIDom()->getBlock() != nodeS.getIDom()->getBlock()) {
    return { "Immediate dominator summary is not correct for: " + pass.suite->printToString(bbS) };
  }

  auto children = node.getChildren();
  auto childrenS = nodeS.getChildren();
  for (auto i = 0; i < node.getNumChildren(); ++i) {
    Values errors = DSTestSuite::domNodeIsIdentical(pass, *children[i], *childrenS[i]);
    if (errors.size() > 0) return errors;
  }
  return {};
}

template <class DTBase>
Values DSTestSuite::domTreeIsIdentical (DSTestSuite &pass, DTBase &DT, DomTreeSummary &DTS) {
  auto &roots = DT.getRoots();
  for (auto root : roots) {
    auto node = DT.getNode(root);
    auto nodeS = DTS.getNode(root);
    if (nodeS == nullptr) return { "Root does not exist!" };
    Values errors = DSTestSuite::domNodeIsIdentical(pass, *node, *nodeS);
    if (errors.size() > 0) return errors;
  }
  return {};
}

Values DSTestSuite::domTreesAreIdentical (ModulePass &pass, TestSuite &suite) {
  auto &dsPass = static_cast<DSTestSuite &>(pass);
  Values errors = DSTestSuite::domTreeIsIdentical(dsPass, *dsPass.dt, dsPass.ds->DT);
  if (errors.size() > 0) return errors;
  errors = DSTestSuite::domTreeIsIdentical(dsPass, *dsPass.pdt, dsPass.ds->PDT);
  if (errors.size() > 0) return errors;
  return {};
}