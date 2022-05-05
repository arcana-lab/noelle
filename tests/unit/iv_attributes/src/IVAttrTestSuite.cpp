/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "IVAttrTestSuite.hpp"
#include "noelle/core/Noelle.hpp"

namespace llvm::noelle {

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
  AU.addRequired<Noelle>();
}

bool IVAttrTestSuite::runOnModule (Module &M) {
  errs() << "IVAttrTestSuite: Start\n";
  auto& noelle = getAnalysis<Noelle>();

  auto mainFunction = M.getFunction("main");

  this->LI = &getAnalysis<LoopInfoWrapperPass>(*mainFunction).getLoopInfo();
  this->SE = &getAnalysis<ScalarEvolutionWrapperPass>(*mainFunction).getSE();

  this->fdg = getAnalysis<PDGAnalysis>().getFunctionPDG(*mainFunction);
  auto topLoop = LI->getLoopsInPreorder()[0];
  auto loopDG = fdg->createLoopsSubgraph(topLoop);
  this->sccdag = new SCCDAG(loopDG);

  /*
   * Fetch the forest node of the loop
   */
  auto allLoopsOfFunction = noelle.getLoopStructures(mainFunction, 0);
  auto forest = noelle.organizeLoopsInTheirNestingForest(*allLoopsOfFunction);
  auto loopNode = forest->getInnermostLoopThatContains(&*topLoop->getHeader()->begin());

  InvariantManager invariantManager(loopNode->getLoop(), loopDG);

  errs() << "IVAttrTestSuite: Running IV analysis\n";
  auto loopExitBlocks = loopNode->getLoop()->getLoopExitBasicBlocks();
  auto environment = new LoopEnvironment(loopDG, loopExitBlocks);
  this->IVs = new InductionVariableManager(loopNode, invariantManager, *SE, *sccdag, *environment, *topLoop);
  errs() << "IVAttrTestSuite: Finished IV analysis\n";

  suite->runTests((ModulePass &)*this);

  delete this->IVs;
  delete sccdag;
  delete loopDG;

  return false;
}

Values IVAttrTestSuite::verifyStartAndStepByLoop (ModulePass &pass, TestSuite &suite) {
  IVAttrTestSuite &attrPass = static_cast<IVAttrTestSuite &>(pass);

  Values loopIVs;
  for (auto loop : attrPass.topLoop->getLoops()) {
    for (auto IV : attrPass.IVs->getInductionVariables(*loop)) {
      std::vector<std::string> loopIVStartStep;
      loopIVStartStep.push_back(suite.printAsOperandToString(loop->getHeader()));
      loopIVStartStep.push_back(suite.valueToString(IV->getStartValue()));

      if (IV->getSingleComputedStepValue()) {
        loopIVStartStep.push_back(suite.valueToString(IV->getSingleComputedStepValue()));
      } else {
        IV->getLoopEntryPHI()->print(errs() << "Header of composite: "); errs() << "\n";
        auto B = (*IV->getComputationOfStepValue().begin())->getParent();
        auto F = IV->getLoopEntryPHI()->getFunction();
        B->insertInto(F);
        for (auto emplacedI : IV->getComputationOfStepValue()) {
          loopIVStartStep.push_back(suite.valueToString(emplacedI));
        }
        B->removeFromParent();
      }

      loopIVs.insert(suite.combineOrderedValues(loopIVStartStep));
    }
  }

  return loopIVs;
}

Values IVAttrTestSuite::verifyIntermediateValues (ModulePass &pass, TestSuite &suite) {
  IVAttrTestSuite &attrPass = static_cast<IVAttrTestSuite &>(pass);

  Values loopIVIntermediates;
  for (auto &loop : attrPass.topLoop->getLoops()) {
    loopIVIntermediates.insert(suite.printAsOperandToString(loop->getHeader()));

    for (auto IV : attrPass.IVs->getInductionVariables(*loop)) {
      std::vector<std::string> intermediates;
      for (auto inst : IV->getAllInstructions()) {
        intermediates.push_back(suite.valueToString(inst));
      }

      loopIVIntermediates.insert(suite.combineUnorderedValues(intermediates));
    }

  }

  return loopIVIntermediates;
}

Values IVAttrTestSuite::verifyLoopGoverning (ModulePass &pass, TestSuite &suite) {
  IVAttrTestSuite &attrPass = static_cast<IVAttrTestSuite &>(pass);

  Values loopGoverningInfos;
  for (auto &loop : attrPass.topLoop->getLoops()) {
    auto IV = attrPass.IVs->getLoopGoverningInductionVariable(*loop);
    if (!IV) continue;

    auto exitBlocks = loop->getLoopExitBasicBlocks();
    auto scc = attrPass.sccdag->sccOfValue(IV->getLoopEntryPHI());
    auto attr = new LoopGoverningIVAttribution(*IV, *scc, exitBlocks);
    if (!attr->isSCCContainingIVWellFormed()) continue;

    std::vector<std::string> info;
    info.push_back(suite.printAsOperandToString(loop->getHeader()));

    std::vector<std::string> startAndStep;
    startAndStep.push_back(suite.valueToString(IV->getStartValue()));
    if (IV->getSingleComputedStepValue()) {
      startAndStep.push_back(suite.valueToString(IV->getSingleComputedStepValue()));
    } else {
      auto B = (*IV->getComputationOfStepValue().begin())->getParent();
      auto F = IV->getLoopEntryPHI()->getFunction();
      B->insertInto(F);
      for (auto emplacedI : IV->getComputationOfStepValue()) {
        startAndStep.push_back(suite.valueToString(emplacedI));
      }
      B->removeFromParent();
    }
    info.push_back(suite.combineOrderedValues(startAndStep));

    info.push_back(suite.valueToString(attr->getHeaderCompareInstructionToComputeExitCondition()));
    info.push_back(suite.valueToString(attr->getHeaderBrInst()));
    info.push_back(suite.valueToString(attr->getExitConditionValue()));

    std::vector<std::string> derivation;
    for (auto value : attr->getConditionValueDerivation()) {
      info.push_back(suite.valueToString(value));
    }
    loopGoverningInfos.insert(suite.combineOrderedValues(info));
  }

  return loopGoverningInfos;
}

}
