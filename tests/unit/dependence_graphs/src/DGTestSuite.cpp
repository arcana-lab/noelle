/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DGTestSuite.hpp"

using namespace llvm;

// Register pass to "opt"
char DGTestSuite::ID = 0;
static RegisterPass<DGTestSuite> X("UnitTester", "Dependence Graph Unit Tester");

// Register pass to "clang"
static DGTestSuite * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new DGTestSuite());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new DGTestSuite());}});// ** for -O0

const char *DGTestSuite::tests[] = {
  "pdg nodes",
  "pdg edges",
  "ldg nodes",
  "pdg root values",
  "pdg leaf values",
  "pdg disjoint values",
};

TestFunction DGTestSuite::testFns[] = {
  DGTestSuite::pdgHasAllValuesInProgram,
  DGTestSuite::pdgHasAllDGEdgesInProgram,
  DGTestSuite::ldgHasOnlyValuesOfLoop,
  DGTestSuite::pdgIdentifiesRootValues,
  DGTestSuite::pdgIdentifiesLeafValues,
  DGTestSuite::pdgIdentifiesDisconnectedValueSets,
};

bool DGTestSuite::doInitialization (Module &M) {
  errs() << "DGTestSuite: Initialize\n";
  const int numTests = sizeof(tests) / sizeof(tests[0]);
  this->suite = new TestSuite("DGTestSuite", tests, testFns, numTests, "test.txt");
  this->M = &M;
  return false;
}

void DGTestSuite::getAnalysisUsage (AnalysisUsage &AU) const {
  AU.addRequired<PDGAnalysis>();
  AU.addRequired<LoopInfoWrapperPass>();
}

bool DGTestSuite::runOnModule (Module &M) {
  errs() << "DGTestSuite: Start\n";

  this->mainF = M.getFunction("main");
  this->fdg = getAnalysis<PDGAnalysis>().getFunctionPDG(*mainF);

  suite->runTests((ModulePass &)*this);

  return false;
}

// Produce expected Values; don't actually expose checkTest
Values DGTestSuite::pdgHasAllValuesInProgram (ModulePass &pass) {
  DGTestSuite &dgPass = static_cast<DGTestSuite &>(pass);
  Values valueNames;
  for (auto node : dgPass.fdg->getNodes()) {
    valueNames.insert(dgPass.suite->printToString(node->getT()));
  }
  return valueNames;
}

Values DGTestSuite::pdgHasAllDGEdgesInProgram (ModulePass &pass) {
  DGTestSuite &dgPass = static_cast<DGTestSuite &>(pass);
  Values valueNames;
  for (auto edge : dgPass.fdg->getEdges()) {
    std::string outName = dgPass.suite->printToString(edge->getOutgoingT());
    std::string inName = dgPass.suite->printToString(edge->getIncomingT());
    std::string type = edge->isControlDependence() ? "control" : (
      edge->isMemoryDependence() ? "memory" : "data"
    );
    std::string delim = dgPass.suite->orderedValueDelimiter;
    valueNames.insert(outName + delim + inName + delim + type);
  }
  return valueNames;
}

Values DGTestSuite::ldgHasOnlyValuesOfLoop (ModulePass &pass) {
  DGTestSuite &dgPass = static_cast<DGTestSuite &>(pass);
  auto &LI = dgPass.getAnalysis<LoopInfoWrapperPass>(*dgPass.mainF).getLoopInfo();
  auto l = LI.getLoopsInPreorder()[0];
  auto ldi = dgPass.fdg->createLoopsSubgraph(l);
  Values valueNames;
  for (auto nodePair : ldi->internalNodePairs()) {
    valueNames.insert(dgPass.suite->printToString(nodePair.first));
  }
  delete ldi;
  return valueNames;
}

Values DGTestSuite::pdgIdentifiesRootValues (ModulePass &pass) {
  DGTestSuite &dgPass = static_cast<DGTestSuite &>(pass);
  Values valueNames;
  for (auto node : dgPass.fdg->getTopLevelNodes()) {
    valueNames.insert(dgPass.suite->printToString(node->getT()));
  }
  return valueNames;
}

Values DGTestSuite::pdgIdentifiesLeafValues (ModulePass &pass) {
  DGTestSuite &dgPass = static_cast<DGTestSuite &>(pass);
  Values valueNames;
  for (auto node : dgPass.fdg->getLeafNodes()) {
    valueNames.insert(dgPass.suite->printToString(node->getT()));
  }
  return valueNames;
}

Values DGTestSuite::pdgIdentifiesDisconnectedValueSets (ModulePass &pass) {
  DGTestSuite &dgPass = static_cast<DGTestSuite &>(pass);
  Values valueSetNames;
  auto disjointSets = dgPass.fdg->getDisconnectedSubgraphs();
  for (auto disjointSet : disjointSets) {
    std::vector<std::string> valueNames;
    for (auto node : *disjointSet) {
      valueNames.push_back(dgPass.suite->printToString(node->getT()));
    }
    std::string setName = valueNames[0];
    for (int i = 1; i < valueNames.size(); ++i) {
      setName += dgPass.suite->unorderedValueDelimiter + valueNames[i];
    }
    valueSetNames.insert(setName);
  }
  return valueSetNames;
}
