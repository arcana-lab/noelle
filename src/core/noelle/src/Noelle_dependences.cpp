/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni
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

namespace llvm::noelle {

PDG *Noelle::getProgramDependenceGraph(void) {
  if (this->programDependenceGraph == nullptr) {
    this->programDependenceGraph = this->pdgAnalysis->getPDG();
  }

  return this->programDependenceGraph;
}

PDG *Noelle::getFunctionDependenceGraph(Function *f) {

  /*
   * Get the PDG
   * The FDG is a subset of it.
   */
  auto pdg = this->getProgramDependenceGraph();

  /*
   * Create the function dependence graph (FDG).
   */
  auto fdg = pdg->createFunctionSubgraph(*f);

  return fdg;
}

std::vector<SCC *> Noelle::sortByHotness(const std::set<SCC *> &SCCs) {
  std::vector<SCC *> s;

  /*
   * Convert the loops into the vector
   */
  for (auto scc : SCCs) {
    s.push_back(scc);
  }

  /*
   * Check if we need to sort
   */
  if (s.size() <= 1) {
    return s;
  }

  /*
   * Fetch the profiles.
   */
  auto hot = this->getProfiles();

  /*
   * Define the order between loops.
   */
  auto compareSCCs = [hot](SCC *s0, SCC *s1) -> bool {
    assert(s0 != nullptr);
    assert(s1 != nullptr);

    /*
     * Fetch the information.
     */
    auto s0Insts = hot->getTotalInstructions(s0);
    auto s1Insts = hot->getTotalInstructions(s1);

    return s0Insts > s1Insts;
  };

  /*
   * Sort the loops.
   */
  std::sort(s.begin(), s.end(), compareSCCs);

  return s;
}

void Noelle::addAnalysis(DependenceAnalysis *a) {
  this->pdgAnalysis->addAnalysis(a);
  this->ldgAnalysis.addAnalysis(a);

  return;
}

} // namespace llvm::noelle
