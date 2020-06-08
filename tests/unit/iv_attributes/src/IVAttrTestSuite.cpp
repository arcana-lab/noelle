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

  this->LI = &getAnalysis<LoopInfoWrapperPass>(*mainFunction).getLoopInfo();
  this->SE = &getAnalysis<ScalarEvolutionWrapperPass>(*mainFunction).getSE();

  this->fdg = getAnalysis<PDGAnalysis>().getFunctionPDG(*mainFunction);
  this->sccdag = new SCCDAG(fdg);

  auto getLLVMLoopFunction = [this](BasicBlock *h) -> Loop *{
    return getAnalysis<LoopInfoWrapperPass>(*h->getParent()).getLoopInfo().getLoopFor(h);
  };
  auto findTripCount = [this](Loop *loopToAnalyze, uint64_t &foundTripCount) -> bool {
    auto tripCount = SE->getSmallConstantTripCount(loopToAnalyze);
    bool hasTripCount = tripCount != 0;
    foundTripCount = hasTripCount ? tripCount : foundTripCount;
    return hasTripCount;
  };

  this->LIS = new LoopsSummary(getLLVMLoopFunction);
  Loop *topLoop = LI->getLoopsInPreorder()[0];
  LIS->populate(*LI, topLoop, findTripCount);

  errs() << "IVAttrTestSuite: Running IV analysis\n";
  this->IVs = new InductionVariables(*LIS, *LI, *SE, *sccdag);
  errs() << "IVAttrTestSuite: Finished IV analysis\n";

  suite->runTests((ModulePass &)*this);

  delete this->IVs;
  delete this->LIS;
  delete sccdag;
  delete fdg;

  return false;
}

Values IVAttrTestSuite::verifyStartAndStepByLoop (ModulePass &pass) {
  IVAttrTestSuite &attrPass = static_cast<IVAttrTestSuite &>(pass);
  TestSuite *suite = attrPass.suite;

  Values loopIVs;
  for (auto &loop : attrPass.LIS->loops) {
    for (auto IV : attrPass.IVs->getInductionVariables(*loop.get())) {
      std::vector<std::string> loopIVStartStep;
      loopIVStartStep.push_back(suite->printAsOperandToString(loop->getHeader()));
      loopIVStartStep.push_back(suite->valueToString(IV->getStartAtHeader()));

      if (IV->getSimpleValueOfStepSize()) {
        loopIVStartStep.push_back(suite->valueToString(IV->getSimpleValueOfStepSize()));
      } else {
        for (auto I : IV->getExpansionOfCompositeStepSize()) {
          loopIVStartStep.push_back(suite->valueToString(I));
        }
      }

      loopIVs.insert(suite->combineOrderedValues(loopIVStartStep));
    }
  }

  return loopIVs;
}

Values IVAttrTestSuite::verifyIntermediateValues (ModulePass &pass) {
  IVAttrTestSuite &attrPass = static_cast<IVAttrTestSuite &>(pass);
  TestSuite *suite = attrPass.suite;

  Values loopIVIntermediates;
  for (auto &loop : attrPass.LIS->loops) {
    loopIVIntermediates.insert(suite->printAsOperandToString(loop->getHeader()));

    for (auto IV : attrPass.IVs->getInductionVariables(*loop.get())) {
      std::vector<std::string> intermediates;
      for (auto inst : IV->getAllInstructions()) {
        intermediates.push_back(suite->valueToString(inst));
      }

      loopIVIntermediates.insert(suite->combineUnorderedValues(intermediates));
    }

  }

  return loopIVIntermediates;
}

Values IVAttrTestSuite::verifyLoopGoverning (ModulePass &pass) {
  IVAttrTestSuite &attrPass = static_cast<IVAttrTestSuite &>(pass);
  TestSuite *suite = attrPass.suite;

  Values loopGoverningInfos;
  for (auto &loop : attrPass.LIS->loops) {
    loopGoverningInfos.insert(suite->printAsOperandToString(loop->getHeader()));

    auto IV = attrPass.IVs->getLoopGoverningInductionVariable(*loop.get());
    if (!IV) continue;

    auto exitBlocks = loop->getLoopExitBasicBlocks();
    auto scc = attrPass.sccdag->sccOfValue(IV->getHeaderPHI());
    auto attr = new LoopGoverningIVAttribution(*IV, *scc, exitBlocks);
    if (!attr->isSCCContainingIVWellFormed()) continue;

    std::vector<std::string> startAndStep;
    startAndStep.push_back(suite->valueToString(IV->getStartAtHeader()));
    if (IV->getSimpleValueOfStepSize()) {
      startAndStep.push_back(suite->valueToString(IV->getSimpleValueOfStepSize()));
    } else {
      for (auto I : IV->getExpansionOfCompositeStepSize()) {
        startAndStep.push_back(suite->valueToString(I));
      }
    }
    loopGoverningInfos.insert(suite->combineOrderedValues(startAndStep));

    loopGoverningInfos.insert(suite->valueToString(attr->getHeaderCmpInst()));
    loopGoverningInfos.insert(suite->valueToString(attr->getHeaderBrInst()));
    loopGoverningInfos.insert(suite->valueToString(attr->getHeaderCmpInstConditionValue()));

    std::vector<std::string> derivation;
    for (auto value : attr->getConditionValueDerivation()) {
      derivation.push_back(suite->valueToString(value));
    }
    if (derivation.size() > 0) loopGoverningInfos.insert(suite->combineOrderedValues(derivation));

  }

  return loopGoverningInfos;
}