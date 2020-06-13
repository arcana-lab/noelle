/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "DGSimplify.hpp"

using namespace llvm;

void DGSimplify::printFnCallGraph () {
  if (this->verbose == Verbosity::Disabled) return;
  for (auto fns : parentFns) {
    errs() << "DGSimplify:   Child function: " << fns.first->getName() << "\n";
    for (auto f : fns.second) {
      errs() << "DGSimplify:   \tParent: " << f->getName() << "\n";
    }
  }
}

void DGSimplify::printFnOrder () {
  if (this->verbose == Verbosity::Disabled) return;
  int count = 0;
  for (auto fn : depthOrderedFns) {
    errs() << "DGSimplify:   Function: " << count++ << " " << fn->getName() << "\n";
  }
}

void DGSimplify::printFnLoopOrder (Function *F) {
  if (this->verbose == Verbosity::Disabled) return;
  auto count = 1;
  for (auto summary : *preOrderedLoops[F]) {
    auto headerBB = summary->getHeader();
    errs() << "DGSimplify:   Loop " << count++ << ", depth: " << summary->getNestingLevel() << "\n";
    // headerBB->print(errs()); errs() << "\n";
  }
}

void DGSimplify::printLoopsToCheck () {
  if (this->verbose == Verbosity::Disabled) return;
  errs() << "DGSimplify:   Loops in checklist ---------------\n";
  for (auto fnLoops : loopsToCheck) {
    auto F = fnLoops.first;
    auto fnInd = fnOrders[F];
    errs() << "DGSimplify:   Fn: "
      << fnInd << " " << F->getName() << "\n";
    auto &allLoops = *preOrderedLoops[F];
    for (auto loop : fnLoops.second) {
      auto loopInd = std::find(allLoops.begin(), allLoops.end(), loop);
      assert(loopInd != allLoops.end() && "DEBUG: Loop not given an order!");
      errs() << "DGSimplify:   \tChecking Loop: " << (loopInd - allLoops.begin()) << "\n";
    }
  }
  errs() << "DGSimplify:   ---------------\n";
}

void DGSimplify::printFnsToCheck () {
  if (this->verbose == Verbosity::Disabled) return;
  errs() << "DGSimplify:   Functions in checklist ---------------\n";
  std::vector<int> fnInds;
  for (auto F : fnsToCheck) fnInds.push_back(fnOrders[F]);
  std::sort(fnInds.begin(), fnInds.end());
  for (auto ind : fnInds) {
    errs() << "DGSimplify:   Fn: "
      << ind << " " << depthOrderedFns[ind]->getName() << "\n";
  }
  errs() << "DGSimplify:   ---------------\n";
}
