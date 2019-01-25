/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"
#include "HELIXTask.hpp"
#include "Architecture.hpp"

using namespace llvm ;

void HELIX::addSynchronizations (
  LoopDependenceInfo *LDI,
  std::vector<SequentialSegment *> *sss
  ){

  /*
   * Iterate over sequential segments.
   */
  for (auto ss : *sss){

    /*
     * We must execute exactly one wait instruction for each sequential segment, for each loop iteration, and for each thread.
     *
     * Create a new variable at the beginning of the iteration.
     * We call this new variable, ssState.
     * This new variable is reponsible to store the information about whether a wait instruction of the current sequential segment has already been executed in the current iteration for the current thread.
     */
    //TODO
    Value *ssState;

    /*
     * Reset the value of ssState at the beginning of the iteration (i.e., loop header)
     */
    //TODO

    /*
     * Define the code that inject wait instructions.
     */
    auto injectWait = [this, ss, ssState](Instruction *justAfterEntry) -> void {

      /*
       * Fetch the sequential segment array
       */
      assert(this->tasks.size() == 1);
      auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);
      auto ssArray = helixTask->ssPastArrayArg;

      /*
       * Compute the offset of the sequential segment entry.
       */
      auto ssID = ss->getID();
      auto ssOffset = ssID * CACHE_LINE_SIZE;

      /*
       * Fetch the pointer to the sequential segment entry.
       */
      //TODO

      /*
       * Check if the ssState has been set already.
       * If it did, then it means that we have already executed wait for "ss" and, therefore, we must not invoke wait another time.
       * If it didn't, then we need to invoke HELIX_wait.
       */
      //TODO

      /*
       * Inject a call to HELIX_wait just before "justAfterEntry"
       */
      //TODO

      /*
       * Set the ssState just after the call to HELIX_wait.
       * This will keep track of the fact that we have executed wait for ss in the current iteration.
       */
      //TODO

      return ;
    };

    /*
     * Define the code that inject wait instructions.
     */
    auto injectSignal = [this, ss](Instruction *justBeforeExit) -> void {

      /*
       * Fetch the sequential segment array
       */
      assert(this->tasks.size() == 1);
      auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);
      auto ssArray = helixTask->ssFutureArrayArg;

      /*
       * Fetch the sequential segment entry.
       */
      auto ssID = ss->getID();
      //TODO

      /*
       * Inject a call to HELIX_signal just after "justBeforeExit" 
       */
      //TODO

      return ;
    };

    /*
     * Inject waits.
     */
    ss->forEachEntry(injectWait);

    /*
     * Inject signals.
     */
    ss->forEachExit(injectSignal);
  }

  return ;
}
