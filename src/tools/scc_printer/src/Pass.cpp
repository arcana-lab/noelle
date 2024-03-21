/*
 * Copyright 2016 - 2020 Simone Campanoni
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
#include "SCCPrinter.hpp"

using namespace llvm;
using namespace arcana::noelle;

static cl::list<int> SCCTypeWhiteList(
    "noelle-scc-printer-white-list",
    cl::ZeroOrMore,
    cl::CommaSeparated,
    cl::desc("Print only a subset of the SCC hierarchy"));
static cl::list<int> SCCTypeBlackList(
    "noelle-scc-printer-black-list",
    cl::ZeroOrMore,
    cl::CommaSeparated,
    cl::desc("Print all but a subset of the SCC hierarchy"));
static cl::opt<bool> PrintSCCInstructions(
    "noelle-scc-printer-insts",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Print all instructions that compose the selected SCCs"));
static cl::opt<bool> PrintLoopIDs(
    "noelle-scc-printer-loops",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Assign and print an incremental ID to loops"));
static cl::opt<int> TargetLoopID("noelle-scc-printer-loop",
                                 cl::ZeroOrMore,
                                 cl::init(-1),
                                 cl::Hidden,
                                 cl::desc("Restrict pass to a single loop"));
static cl::opt<bool> PrintDetails(
    "noelle-scc-printer-details",
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Print detailed info about each SCC"));
static cl::opt<std::string> TargetFunctionName(
    "noelle-scc-printer-func",
    cl::init("main"),
    cl::ZeroOrMore,
    cl::Hidden,
    cl::desc("Restrict pass to a single function"));

bool SCCPrinter::doInitialization(Module &M) {
  this->sccTypeWhiteList = SCCTypeWhiteList;
  this->sccTypeBlackList = SCCTypeBlackList;
  this->printSCCInstructions = PrintSCCInstructions;
  this->targetLoopID = TargetLoopID;
  this->targetFunctionName = TargetFunctionName;
  this->loopIDs = PrintLoopIDs;
  this->printDetails = PrintDetails;

  return false;
}

void SCCPrinter::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<Noelle>();
  return;
}

// Next there is code to register your pass to "opt"
char SCCPrinter::ID = 0;
static RegisterPass<SCCPrinter> X("scc-printer", "SCCPrinter");

// Next there is code to register your pass to "clang"
static SCCPrinter *_PassMaker = NULL;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
                                        [](const PassManagerBuilder &,
                                           legacy::PassManagerBase &PM) {
                                          if (!_PassMaker) {
                                            PM.add(_PassMaker =
                                                       new SCCPrinter());
                                          }
                                        }); // ** for -Ox
static RegisterStandardPasses _RegPass2(
    PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {
      if (!_PassMaker) {
        PM.add(_PassMaker = new SCCPrinter());
      }
    }); // ** for -O0
