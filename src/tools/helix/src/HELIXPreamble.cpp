/*
 * Copyright 2021 - 2022  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"
#include "HELIXTask.hpp"

namespace llvm::noelle{
      
bool HELIX::doesHaveASequentialPreamble (
  LoopDependenceInfo *LDI
  ) const {

  /*
   * Fetch the loop SCCDAG
   */
  auto sccManager = LDI->getSCCManager();
  auto loopSCCDAG = sccManager->getSCCDAG();

  /*
   * Fetch the source nodes in the loop SCCDAG
   */
  auto preambleSCCNodes = loopSCCDAG->getTopLevelNodes();
  if (preambleSCCNodes.size() != 1){

    /*
     * If we have more than one, then we don't have a preamble.
     */
    return false;
  }

  /*
   * Fetch the single source node.
   */
  auto preambleSCC = (*preambleSCCNodes.begin())->getT();
  assert(preambleSCC != nullptr);

  /*
   * Check the SCC to see if it has to run sequentially
   */
  auto sccInfo = sccManager->getSCCAttrs(preambleSCC);
  if (     true
        && (!sccInfo->isInductionVariableSCC())
        && sccInfo->mustExecuteSequentially()
      ){

    /*
     * The SCC must execute sequentially.
     *
     * Check if the SCC has the header in it.
     * If it doesn't, then we don't have a preamble.
     */
    auto loopStructure = LDI->getLoopStructure();
    auto loopHeader = loopStructure->getHeader();
    auto foundHeader = false;
    for (auto p : loopStructure->getLoopExitEdges()){
      auto eN = p.first;
      auto e = eN->getTerminator();
    }
    for (auto instNode : preambleSCC->getNodes()){
      auto inst = cast<Instruction>(instNode->getT());
      if (loopStructure->isALoopExit(inst)){

        /*
         * This loop has a sequential preamble.
         */
        return true;
      }
    }
  }
 
  /*
   * This loop does not have a sequential preamble.
   */
  return false;
}

}
