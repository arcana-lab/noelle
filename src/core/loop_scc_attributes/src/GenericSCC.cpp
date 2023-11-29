/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
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
#include "noelle/core/GenericSCC.hpp"

namespace arcana::noelle {

GenericSCC::GenericSCC(SCCKind K, SCC *s, LoopStructure *loop)
  : loop{ loop },
    scc{ s },
    PHINodes{},
    kind{ K },
    hasMemoryDependences{ false } {

  /*
   * Collect PHIs included in the SCC.
   */
  this->collectPHIs(*loop);

  /*
   * Check if the SCC has memory dependences.
   */
  for (auto edge : this->scc->getEdges()) {
    if (edge->isMemoryDependence()) {
      this->hasMemoryDependences = true;
      break;
    }
  }

  return;
}

bool GenericSCC::doesHaveMemoryDependencesWithin(void) const {
  return this->hasMemoryDependences;
}

iterator_range<GenericSCC::phi_iterator> GenericSCC::getPHIs(void) const {
  return make_range(this->PHINodes.begin(), this->PHINodes.end());
}

void GenericSCC::collectPHIs(LoopStructure &LS) {

  /*
   * Iterate over elements of the SCC to collect PHIs.
   */
  for (auto iNodePair : this->scc->internalNodePairs()) {

    /*
     * Fetch the current element of the SCC.
     */
    auto V = iNodePair.first;

    /*
     * Check if it is a PHI.
     */
    if (auto phi = dyn_cast<PHINode>(V)) {
      this->PHINodes.insert(phi);
    }
  }

  return;
}

SCC *GenericSCC::getSCC(void) {
  return this->scc;
}

GenericSCC::SCCKind GenericSCC::getKind(void) const {
  return this->kind;
}

GenericSCC::~GenericSCC() {}

} // namespace arcana::noelle
