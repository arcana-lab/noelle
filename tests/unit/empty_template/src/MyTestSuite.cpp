/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "MyTestSuite.hpp"

using namespace llvm;

// Register pass to "opt"
char MyTestSuite::ID = 0;
static RegisterPass<MyTestSuite> X("UnitTester", "Template Unit Tester");

// Register pass to "clang"
static MyTestSuite * _PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new MyTestSuite());}}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new MyTestSuite());}});// ** for -O0

const char *MyTestSuite::tests[] = {
};
TestFunction MyTestSuite::testFns[] = {
};

bool MyTestSuite::doInitialization (Module &M) {
  errs() << "MyTestSuite: Initialize\n";
  const int numTests = sizeof(tests) / sizeof(tests[0]);
  this->suite = new TestSuite("MyTestSuite", tests, testFns, numTests, "test.txt");
  this->M = &M;
  return false;
}

void MyTestSuite::getAnalysisUsage (AnalysisUsage &AU) const {
  AU.addRequired<PDGAnalysis>();
}

bool MyTestSuite::runOnModule (Module &M) {
  errs() << "MyTestSuite: Start\n";
  auto mainFunction = M.getFunction("main");
  auto fdg = getAnalysis<PDGAnalysis>().getFunctionPDG(*mainFunction);

  suite->runTests((ModulePass &)*this);

  delete fdg;

  return false;
}