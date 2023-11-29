/*
 * Copyright 2021 - 2022  Simone Campanoni
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
#include "noelle/core/CFGAnalysis.hpp"
#include "noelle/core/DataFlowAnalysis.hpp"

namespace arcana::noelle {

CFGAnalysis::CFGAnalysis() : ModulePass{ ID } {
  return;
}

bool CFGAnalysis::isIncludedInACycle(BasicBlock &bb) {

  /*
   * Fetch an instruction of the basic block
   */
  auto inst = &*bb.begin();

  /*
   * Check if that instruction is included in a cycle.
   */
  auto cycle = this->isIncludedInACycle(*inst);

  return cycle;
}

bool CFGAnalysis::isIncludedInACycle(Instruction &i) {

  /*
   * Compute the reachable analysis.
   */
  DataFlowAnalysis dfa{};
  auto f = i.getFunction();
  auto filter = [&i](Instruction *inst) -> bool {
    if (inst == &i) {
      return true;
    }
    return false;
  };
  auto dfr = dfa.runReachableAnalysis(f, filter);

  /*
   * Check if the instruction @i is reachable just after it.
   * If it is, then @i is within a cycle.
   */
  auto &outSet = dfr->OUT(&i);
  if (outSet.count(&i) > 0) {
    return true;
  }

  return false;
}

} // namespace arcana::noelle
