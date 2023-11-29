/*
 * Copyright 2023 - Federico Sossai, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "PlanInfo.hpp"

static cl::opt<bool> PrintAllHeaders(
    "info-print-all-headers",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Print the header of all loops with a parallel plan"));
static cl::list<int> PrintHeaders(
    "info-print-headers",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::CommaSeparated,
    cl::desc("Print the headers of some loops with a parallel plan"));

namespace arcana::noelle {

bool PlanInfo::doInitialization(Module &M) {
  this->printAllHeaders = PrintAllHeaders.getValue();
  this->printHeaders = PrintHeaders;
  return false;
}

void PlanInfo::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<Noelle>();
  return;
}

// Next there is code to register your pass to "opt"
char PlanInfo::ID = 0;
static RegisterPass<PlanInfo> X("ParallelizerPlanInfo",
                                "Print information about a parallel plan");

// Next there is code to register your pass to "clang"
static PlanInfo *_PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
                                        [](const PassManagerBuilder &,
                                           legacy::PassManagerBase &PM) {
                                          if (!_PassMaker) {
                                            PM.add(_PassMaker = new PlanInfo());
                                          }
                                        }); // ** for -Ox
static RegisterStandardPasses _RegPass2(
    PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
      if (!_PassMaker) {
        PM.add(_PassMaker = new PlanInfo());
      }
    }); // ** for -O0

} // namespace arcana::noelle
