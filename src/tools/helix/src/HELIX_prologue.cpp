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
#include "noelle/core/InductionVariableSCC.hpp"
#include "noelle/core/LoopCarriedUnknownSCC.hpp"
#include "noelle/tools/HELIX.hpp"

namespace arcana::noelle {

bool HELIX::doesHaveASequentialPrologue(LoopContent *LDI) const {

  /*
   * Fetch the loop governing IV.
   */
  auto IVManager = LDI->getInductionVariableManager();
  auto loopGoverningIVAttr = IVManager->getLoopGoverningInductionVariable();
  if (!loopGoverningIVAttr) {

    /*
     * The loop does not have a loop governing IV.
     * Hence, this loop must have a sequential prologue.
     */
    return true;
  }

  /*
   * Fetch the sequential SCC that creates the sequential prologue.
   */
  auto seqSCC = this->getTheSequentialSCCThatCreatesTheSequentialPrologue(LDI);
  if (seqSCC == nullptr) {
    return false;
  }

  return true;
}

SCC *HELIX::getTheSequentialSCCThatCreatesTheSequentialPrologue(
    LoopContent *LDI) const {

  /*
   * Fetch the loop SCCDAG
   */
  auto sccManager = LDI->getSCCManager();
  auto loopSCCDAG = sccManager->getSCCDAG();

  /*
   * Fetch the source nodes in the loop SCCDAG
   */
  auto preambleSCCNodes = loopSCCDAG->getTopLevelNodes();
  if (preambleSCCNodes.size() == 0) {

    /*
     * If we have more than one, then we don't have a preamble.
     */
    return nullptr;
  }
  assert(preambleSCCNodes.size() == 1);

  /*
   * Fetch the single source node.
   */
  auto preambleSCC = (*preambleSCCNodes.begin())->getT();
  assert(preambleSCC != nullptr);

  /*
   * Check the SCC to see if it has to run sequentially
   */
  auto sccInfo = sccManager->getSCCAttrs(preambleSCC);
  if (isa<LoopCarriedUnknownSCC>(sccInfo)) {

    /*
     * The SCC must execute sequentially.
     *
     * Check if the SCC has the header in it.
     * If it doesn't, then we don't have a preamble.
     */
    auto loopStructure = LDI->getLoopStructure();
    auto loopHeader = loopStructure->getHeader();
    auto foundHeader = false;
    for (auto instNode : preambleSCC->getNodes()) {
      auto inst = cast<Instruction>(instNode->getT());
      if (loopStructure->isALoopExit(inst)) {

        /*
         * This loop has a sequential preamble.
         */
        return preambleSCC;
      }
    }
  }

  /*
   * This loop does not have a sequential preamble.
   */
  return nullptr;
}

} // namespace arcana::noelle
