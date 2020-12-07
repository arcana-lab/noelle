/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"

#include "HotProfiler.hpp"

using namespace llvm;
using namespace llvm::noelle;

void HotProfiler::analyzeProfiles (Module &M){

  /*
   * Fetch the invocations of each basic block of each function.
   */
  for (auto &F : M){
    if (F.empty()){
      continue ;
    }
    auto& bfi = getAnalysis<BlockFrequencyInfoWrapperPass>(F).getBFI();
    auto& bpi = getAnalysis<BranchProbabilityInfoWrapperPass>(F).getBPI();

    /*
     * Set the invocations of basic blocks.
     */
    for (auto& bb : F){

      /*
       * Check if the basic block has been executed at least once.
       */
      if (!bfi.getBlockProfileCount(&bb).hasValue()) {

        /*
         * The basic block hasn't been executed.
         */
        this->hot.setBasicBlockInvocations(&bb, 0);
        continue;
      }

      /*
       * Fetch the basic block counter.
       */
      auto v = bfi.getBlockProfileCount(&bb).getValue();

      /*
       * Set the invocations.
       */
      this->hot.setBasicBlockInvocations(&bb, v);

      /*
       * Compute the frequency of jumping to the successors of bb.
       */
      for (auto succBB : successors(&bb)){
        auto prob = bpi.getEdgeProbability(&bb, succBB);
        if (prob.isUnknown()){
          continue ;
        }
        auto probNum = double(prob.getNumerator());
        auto probDen = double(prob.getDenominator());
        auto probValue = probNum / probDen;

        /*
         * Set the frequency.
         */
        this->hot.setBranchFrequency(&bb, succBB, probValue);
      }
    }
  }

  /*
   * Compute the global counters.
   */
  this->hot.computeProgramInvocations(M);

  return ;
}

Hot& HotProfiler::getHot (void){
  return this->hot;
}
