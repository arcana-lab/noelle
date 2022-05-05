/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LICMTestSuite.hpp"

namespace llvm::noelle {

// Register pass to "opt"
char LICMTestSuite::ID = 0;
static RegisterPass<LICMTestSuite> X("UnitTester", "Loop Invariant Code Motion Unit Tester");

// Register pass to "clang"
static LICMTestSuite * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new LICMTestSuite());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new LICMTestSuite());}});// ** for -O0

const char *LICMTestSuite::tests[] = {
  "loads and stores are hoisted"
};

TestFunction LICMTestSuite::testFns[] = {
  LICMTestSuite::loadsAndStoresAreHoistedFromLoop
};

bool LICMTestSuite::doInitialization (Module &M) {
  errs() << "LICMTestSuite: Initialize\n";
  const int numTests = sizeof(tests) / sizeof(tests[0]);
  this->suite = new TestSuite("LICMTestSuite", tests, testFns, numTests, "test.txt");
  this->M = &M;
  return false;
}

void LICMTestSuite::getAnalysisUsage (AnalysisUsage &AU) const {
  AU.addRequired<Noelle>();
  AU.addRequired<PDGAnalysis>();
  AU.addRequired<LoopInfoWrapperPass>();
  AU.addRequired<DominatorTreeWrapperPass>();
  AU.addRequired<PostDominatorTreeWrapperPass>();
  AU.addRequired<ScalarEvolutionWrapperPass>();
}

bool LICMTestSuite::runOnModule (Module &M) {
  errs() << "LICMTestSuite: Start\n";

  this->mainF = M.getFunction("main");
  auto& noelle = getAnalysis<Noelle>();
  this->fdg = getAnalysis<PDGAnalysis>().getFunctionPDG(*mainF);
  auto &LI = getAnalysis<LoopInfoWrapperPass>(*mainF).getLoopInfo();
  auto& DT = getAnalysis<DominatorTreeWrapperPass>(*mainF).getDomTree();
  auto& PDT = getAnalysis<PostDominatorTreeWrapperPass>(*mainF).getPostDomTree();
  auto& SE = getAnalysis<ScalarEvolutionWrapperPass>(*mainF).getSE();

  /*
   * Fetch the LLVM loop
   */
  auto l = LI.getLoopsInPreorder()[0];

  /*
   * Fetch the forest node of the loop
   */
  auto allLoopsOfFunction = noelle.getLoopStructures(mainF, 0);
  auto forest = noelle.organizeLoopsInTheirNestingForest(*allLoopsOfFunction);
  auto loopNode = forest->getInnermostLoopThatContains(&*l->getHeader()->begin());

  errs() << "LICMTestSuite: Instantiating LDI and LoopInvariantCodeMotion components\n";
  DominatorSummary DS{DT, PDT};
  auto om = noelle.getCompilationOptionsManager();
  this->ldi = new LoopDependenceInfo(fdg, loopNode, l, DS, SE, om->getMaximumNumberOfCores(), true, false);
  this->licm = new LoopInvariantCodeMotion(noelle);

  // PDGPrinter pdgPrinter;
  // pdgPrinter.printGraphsForFunction(*mainF, fdg, LI);
  // SCCDAG sccdag(loopDG);
  // fdg->print(errs() << "FDG of main:\n") << "\n";

  errs() << "LICMTestSuite: Running tests\n";
  suite->runTests((ModulePass &)*this);

  errs() << "LICMTestSuite: Freeing memory\n";
  delete this->licm;
  delete this->ldi;
  delete this->suite;

  return false;
}

Values LICMTestSuite::loadsAndStoresAreHoistedFromLoop (ModulePass &pass, TestSuite &suite) {
  auto &licmPass = static_cast<LICMTestSuite &>(pass);

  /*
   * Collect values before promotion
   */
  auto ldi = licmPass.ldi;
  auto loopStructure = ldi->getLoopStructure();
  std::unordered_set<Instruction *> memoryInsts{};
  std::unordered_map<Instruction *, std::string> memoryInstPrints;
  for (auto loopInst : loopStructure->getInstructions()) {
    if (isa<StoreInst>(loopInst) || isa<LoadInst>(loopInst)) {
      memoryInsts.insert(loopInst);
      memoryInstPrints.insert(std::make_pair(loopInst, suite.printToString(loopInst)));
    }
  }

  licmPass.licm->promoteMemoryLocationsToRegisters(*ldi);

  /*
   * Collect remaining values after promotion
   */
  std::unordered_set<Instruction *> remainingMemoryInsts{};
  for (auto loopInst : loopStructure->getInstructions()) {
    if (isa<StoreInst>(loopInst) || isa<LoadInst>(loopInst)) {
      remainingMemoryInsts.insert(loopInst);
    }
  }

  /*
   * Filter values that still exist
   */
  Values hoistedValues;
  std::unordered_set<Instruction *> hoistedMemoryInsts{};
  for (auto memoryInst : memoryInsts) {
    if (remainingMemoryInsts.find(memoryInst) != remainingMemoryInsts.end()) continue;
    hoistedValues.insert(memoryInstPrints.at(memoryInst));
  }

  return hoistedValues;
}

}
