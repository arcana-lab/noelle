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
  auto pcf = fm->getProgramCallGraph();
  auto mayPointToAnalysis = noelle.getMayPointToAnalysis();

  auto ptSum = mayPointToAnalysis.getPointToSummary(M, pcf);

  setStackMemoryUsage(ptSum);

  auto h2s = collectHeapToStack(noelle, ptSum);

  auto g2s = collectGlobalToStack(noelle, ptSum);

  for (auto &[f, liveMemSum] : h2s) {
    modified |= applyHeapToStack(noelle, liveMemSum);
  }

  for (auto &[globalVar, privariableFunctions] : g2s) {
    modified |= applyGlobalToStack(noelle, globalVar, privariableFunctions);
  }

  return modified;
}

} // namespace llvm::noelle
