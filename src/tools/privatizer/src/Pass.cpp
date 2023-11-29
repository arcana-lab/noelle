/*
 * Copyright 2023 Xiao Chen
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
#include "Privatizer.hpp"

namespace llvm::noelle {

static cl::opt<bool> DisablePrivatizer("noelle-disable-privatizer",
                                       cl::ZeroOrMore,
                                       cl::Hidden,
                                       cl::desc("Disable all privatizers"));

bool Privatizer::doInitialization(Module &M) {
  this->M = &M;

  this->enablePrivatizer =
      (DisablePrivatizer.getNumOccurrences() == 0) ? true : false;

  return false;
}

void Privatizer::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<Noelle>();
  return;
}

// Next there is code to register your pass to "opt"
char Privatizer::ID = 0;
static RegisterPass<Privatizer> X(
    "privatizer",
    "Transforms to turn @malloc() / @calloc() and global variables to allocaInst");

} // namespace llvm::noelle
