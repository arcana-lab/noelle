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
#include "noelle/core/Noelle.hpp"
#include "PrivatizerManager.hpp"

namespace llvm::noelle {

PrivatizerManager::PrivatizerManager() : ModulePass{ ID } {
  return;
}

bool PrivatizerManager::runOnModule(Module &M) {

  /*
   * Check if enablers have been enabled.
   */
  if (!this->enablePrivatizer) {
    return false;
  }
  auto prefix = "PrivatizerManager: ";
  errs() << prefix << "Start\n";

  /*
   * Fetch NOELLE.
   */
  auto &noelle = getAnalysis<Noelle>();

  auto modified = false;

  auto fm = noelle.getFunctionsManager();
  auto mainF = fm->getEntryFunction();
  auto mayPointToAnalysis = noelle.getMayPointToAnalysis();
  auto ptSum = mayPointToAnalysis.getPointToSummary(M);

  auto pcf = fm->getProgramCallGraph();
  auto islands = pcf->getIslands();
  auto islandOfMain = islands[mainF];
  for (auto &F : M) {
    auto funcSum = ptSum->getFunctionSummary(&F);
    auto fname = F.getName();
    errs()
        << prefix
        << "Try to transform @malloc() or @calloc() to allocaInst in function "
        << fname << ".\n";
    auto h2s = this->applyHeapToStack(ptSum, funcSum);
    errs() << prefix << (h2s ? "" : "no ")
           << "@malloc() or @calloc() transformed to allocaInst in function "
           << fname << ".\n";
    modified |= h2s;

    // errs() << prefix << "Try to transform global variables to allocaInst in
    // function " << fname << ".\n"; auto g2s = this->applyGlobalToStack(noelle,
    // mayPointToAnalysis); errs() << prefix << (g2s ? "" : "no ")
    //        << "global variables transformed to allocaInst in function " <<
    //        fname << ".\n";
    // modified |= g2s;
  }

  errs() << prefix << "Exit\n";
  return modified;
}

} // namespace llvm::noelle
