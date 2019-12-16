/*
 * Copyright 2019  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Parallelizer.hpp"

using namespace llvm;

bool Parallelizer::applyEnablers (
    LoopDependenceInfo *LDI,
    Parallelization &par,
    LoopDistribution &loopDist
  ){

  /*
   * Fetch the SCCDAG of the loop.
   */
  auto SCCDAG = LDI->sccdagAttrs.getSCCDAG();
  errs() << "XAN: INOUT: Start\n";
  SCCDAG->iterateOverLiveInAndLiveOut([](Instruction *v) -> bool{
      errs() << "XAN: INTOUT:   " << *v << "\n";
      return false;
      });
  errs() << "XAN: INOUT: Exit\n";

  /*
   * Define the set of SCCs to bring outside the loop.
   */
  std::set<SCC *> SCCsToBringOutsideParallelizedLoop{};

  /*
   * Collect all sequential SCCs.
   */
  std::set<SCC *> sequentialSCCs{};
  auto collectSequentialSCCsFunction = [LDI,&sequentialSCCs](SCC *currentSCC) -> bool {

    /*
     * Fetch the SCC metadata.
     */
    auto sccInfo = LDI->sccdagAttrs.getSCCAttrs(currentSCC);
    errs() << "XAN: Start\n";
    errs() << "XAN:   Number of instructions = " << currentSCC->numberOfInstructions() << "\n";
    currentSCC->iterateOverInstructions([](Instruction *i) -> bool {
        errs() << "XAN:   " << *i << "\n";
        return false;
        });
    errs() << "XAN: Exit\n";
    currentSCC->print(errs());

    /*
     * Check if the current SCC can be removed (e.g., because it is due to induction variables).
     * If it is, then we do not need to remove it from the loop to be parallelized.
     */
    if (!sccInfo->mustExecuteSequentially()) {
      return false;
    }

    /*
     * The current SCC must run sequentially.
     */
    sequentialSCCs.insert(currentSCC);
    return false;
  } ;
  SCCDAG->iterateOverSCCs(collectSequentialSCCsFunction);

  /*
   * Check every sequential SCC of the loop and decide which ones to bring outside the loop to parallelize.
   */
  for (auto SCC : sequentialSCCs){

    /*
     * Try to bring the sequential SCC outside the loop.
     */
    std::set<Instruction *> instsRemoved;
    std::set<Instruction *> instsAdded;
    auto splitted = loopDist.splitLoop(*LDI, SCC, instsRemoved, instsAdded);
    if (!splitted){
      continue ;
    }

    /*
     * The SCC has been pulled out the loop.
     * We need to update all metadata about loops.
     * To do so, we currently quit and rerun NOELLE.
     */
    return true;
  }
  
  return false;
}
