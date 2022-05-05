/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/Noelle.hpp"
#include "SCCDAGAttrTestSuite.hpp"

namespace llvm::noelle {

// Register pass to "opt"
char SCCDAGAttrTestSuite::ID = 0;
static RegisterPass<SCCDAGAttrTestSuite> X("UnitTester", "SCCDAG Attribute Unit Tester");

// Register pass to "clang"
static SCCDAGAttrTestSuite * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new SCCDAGAttrTestSuite());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new SCCDAGAttrTestSuite());}});// ** for -O0

const char *SCCDAGAttrTestSuite::tests[] = {
  "sccdag nodes",
  "scc with IV",
  "reducible SCC",
  "clonable SCC",
  "clonable SCC into local memory",
  "loop carried dependencies (top loop)"
};
TestFunction SCCDAGAttrTestSuite::testFns[] = {
  SCCDAGAttrTestSuite::sccdagHasCorrectSCCs,
  SCCDAGAttrTestSuite::sccsWithIVAreFound,
  SCCDAGAttrTestSuite::reducibleSCCsAreFound,
  SCCDAGAttrTestSuite::clonableSCCsAreFound,
  SCCDAGAttrTestSuite::clonableSCCsIntoLocalMemoryAreFound,
  SCCDAGAttrTestSuite::loopCarriedDependencies
};

bool SCCDAGAttrTestSuite::doInitialization (Module &M) {
  errs() << "SCCDAGAttrTestSuite: Initialize\n";
  const int numTests = sizeof(tests) / sizeof(tests[0]);
  this->suite = new TestSuite("SCCDAGAttrTestSuite", tests, testFns, numTests, "test.txt");
  this->M = &M;
  return false;
}

void SCCDAGAttrTestSuite::getAnalysisUsage (AnalysisUsage &AU) const {
  AU.addRequired<PDGAnalysis>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<CallGraphWrapperPass>();
  AU.addRequired<Noelle>();
}

bool SCCDAGAttrTestSuite::runOnModule (Module &M) {
  errs() << "SCCDAGAttrTestSuite: Start\n";
  auto& noelle = getAnalysis<Noelle>();

  auto mainFunction = M.getFunction("main");

  this->LI = &getAnalysis<LoopInfoWrapperPass>(*mainFunction).getLoopInfo();
  this->SE = &getAnalysis<ScalarEvolutionWrapperPass>(*mainFunction).getSE();

  // TODO: Grab first loop and produce attributes on it
  auto topLoop = LI->getLoopsInPreorder()[0];

  /*
   * Fetch the dominators
   */
  auto DS = noelle.getDominators(mainFunction);

  /*
   * Fetch the forest node of the loop
   */
  auto allLoopsOfFunction = noelle.getLoopStructures(mainFunction, 0);
  auto forest = noelle.organizeLoopsInTheirNestingForest(*allLoopsOfFunction);
  auto loopNode = forest->getInnermostLoopThatContains(&*topLoop->getHeader()->begin());

  this->fdg = getAnalysis<PDGAnalysis>().getFunctionPDG(*mainFunction);
  auto loopDI = new LoopDependenceInfo(fdg, loopNode, topLoop, *DS, *SE, 2, true, true);
  auto sccManager = loopDI->getSCCManager();

  this->sccdag = sccManager->getSCCDAG();

  errs() << "SCCDAGAttrTestSuite: Constructing IVAttributes\n";
  auto IV = loopDI->getInvariantManager();

  errs() << "SCCDAGAttrTestSuite: Constructing SCCDAGAttrs\n";

  // TODO: Test attribution on normalized SCCDAG as well
  this->attrs = sccManager;

  // DGPrinter::writeGraph<SCCDAG, SCC>("graph-loop.dot", sccdag);

  errs() << "SCCDAGAttrTestSuite: Running suite\n";
  suite->runTests((ModulePass &)*this);

  delete this->attrs;
  delete this->sccdag;

  return false;
}

Values SCCDAGAttrTestSuite::sccdagHasCorrectSCCs (ModulePass &pass, TestSuite &suite) {
  SCCDAGAttrTestSuite &attrPass = static_cast<SCCDAGAttrTestSuite &>(pass);
  return attrPass.getValuesOfSCCDAG(*attrPass.sccdag);
}

Values SCCDAGAttrTestSuite::getValuesOfSCCDAG (SCCDAG &dag) {
  Values valueNames;
  for (auto node : dag.getNodes()) {
    std::vector<std::string> sccValues;
    for (auto nodePair : node->getT()->internalNodePairs()) {
      sccValues.push_back(suite->valueToString(nodePair.first));
    }
    valueNames.insert(TestSuite::combineValues(sccValues, suite->unorderedValueDelimiter));
  }
  return valueNames;
}

Values SCCDAGAttrTestSuite::sccsWithIVAreFound (ModulePass &pass, TestSuite &suite) {
  SCCDAGAttrTestSuite &attrPass = static_cast<SCCDAGAttrTestSuite &>(pass);
  std::set<SCC *> sccs;
  for (auto node : attrPass.sccdag->getNodes()) {
    SCCAttrs *sccAttrs = attrPass.attrs->getSCCAttrs(node->getT());
    if (sccAttrs->isInductionVariableSCC()) sccs.insert(node->getT());
  }

  return SCCDAGAttrTestSuite::printSCCs(pass, suite, sccs);
}

Values SCCDAGAttrTestSuite::reducibleSCCsAreFound (ModulePass &pass, TestSuite &suite) {
  SCCDAGAttrTestSuite &attrPass = static_cast<SCCDAGAttrTestSuite &>(pass);
  std::set<SCC *> sccs;
  for (auto node : attrPass.sccdag->getNodes()) {
    SCCAttrs *sccAttrs = attrPass.attrs->getSCCAttrs(node->getT());
    if (sccAttrs->canExecuteReducibly()) sccs.insert(node->getT());
  }

  return SCCDAGAttrTestSuite::printSCCs(pass, suite, sccs);
}

Values SCCDAGAttrTestSuite::clonableSCCsAreFound (ModulePass &pass, TestSuite &suite) {
  SCCDAGAttrTestSuite &attrPass = static_cast<SCCDAGAttrTestSuite &>(pass);
  std::set<SCC *> sccs;
  for (auto node : attrPass.sccdag->getNodes()) {
    SCCAttrs *sccAttrs = attrPass.attrs->getSCCAttrs(node->getT());
    if (sccAttrs->canBeCloned()) sccs.insert(node->getT());
  }

  return SCCDAGAttrTestSuite::printSCCs(pass, suite, sccs);
}

Values SCCDAGAttrTestSuite::clonableSCCsIntoLocalMemoryAreFound (ModulePass &pass, TestSuite &suite) {
  auto &attrPass = static_cast<SCCDAGAttrTestSuite &>(pass);

  std::set<SCC *> sccs;
  for (auto node : attrPass.sccdag->getNodes()) {
    auto scc = node->getT();
    auto sccAttrs = attrPass.attrs->getSCCAttrs(scc);
    if (sccAttrs->canBeClonedUsingLocalMemoryLocations()) {
      sccs.insert(node->getT());
    }
  }

  return SCCDAGAttrTestSuite::printSCCs(pass, suite, sccs);
}

Values SCCDAGAttrTestSuite::printSCCs (ModulePass &pass, TestSuite &suite, std::set<SCC *> sccs) {
  auto &attrPass = static_cast<SCCDAGAttrTestSuite &>(pass);
  Values valueNames{};
  for (auto scc : sccs) {
    std::vector<std::string> sccValues;
    for (auto nodePair : scc->internalNodePairs()) {
      sccValues.push_back(suite.valueToString(nodePair.first));
    }
    valueNames.insert(TestSuite::combineValues(sccValues, suite.unorderedValueDelimiter));
  }

  return valueNames;
}

Values SCCDAGAttrTestSuite::loopCarriedDependencies (ModulePass &pass, TestSuite &suite) {
  SCCDAGAttrTestSuite &attrPass = static_cast<SCCDAGAttrTestSuite &>(pass);
  Values valueNames{};
  for (auto sccAndDeps : attrPass.attrs->sccToLoopCarriedDependencies) {
    for (auto dep : sccAndDeps.second) {
      std::string outValue = suite.valueToString(dep->getOutgoingT());
      std::string inValue = suite.valueToString(dep->getIncomingT());
      valueNames.insert(outValue + suite.orderedValueDelimiter + inValue);
    }
  }

  return valueNames;
}

}
