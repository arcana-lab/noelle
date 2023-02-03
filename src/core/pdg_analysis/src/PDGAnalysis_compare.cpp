/*
 * Copyright 2016 - 2020  Angelo Matni, Yian Su, Simone Campanoni
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
#include "noelle/core/SystemHeaders.hpp"

#include "noelle/core/PDGPrinter.hpp"
#include "noelle/core/PDGAnalysis.hpp"

using namespace llvm;
using namespace llvm::noelle;

bool PDGAnalysis::comparePDGs(PDG *pdg1, PDG *pdg2) {
  return compareNodes(pdg1, pdg2) && compareEdges(pdg1, pdg2);
}

bool PDGAnalysis::compareNodes(PDG *pdg1, PDG *pdg2) {
  errs() << "Compare PDG Nodes\n";

  if (pdg1->numNodes() != pdg2->numNodes()) {
    errs() << " number of pdg nodes are not the same\n";
    return false;
  }

  for (auto &node : pdg1->getNodes()) {
    if (pdg2->fetchNode(node->getT()) == nullptr) {
      return false;
    }
  }

  return true;
}

bool PDGAnalysis::compareEdges(
    PDG *pdg1,
    PDG *pdg2,
    std::function<void(DGEdge<Value> *dependenceMissingInPdg2)> func) {
  for (auto &edge1 : pdg1->getEdges()) {
    auto edgeSet =
        pdg2->getDependences(edge1->getOutgoingT(), edge1->getIncomingT());
    if (edgeSet.empty()) {
      func(edge1);
      return false;
    }

    auto match = false;
    for (auto &edge2 : edgeSet) {
      if (edge1->isMemoryDependence() == edge2->isMemoryDependence()
          && edge1->isMustDependence() == edge2->isMustDependence()
          && edge1->isControlDependence() == edge2->isControlDependence()
          && edge1->isLoopCarriedDependence()
                 == edge2->isLoopCarriedDependence()
          && edge1->isRemovableDependence() == edge2->isRemovableDependence()
          && edge1->dataDependenceType() == edge2->dataDependenceType()) {
        match = true;
        break;
      }
    }
    if (!match) {
      func(edge1);
      return false;
    }
  }

  return true;
}

bool PDGAnalysis::compareEdges(PDG *pdg1, PDG *pdg2) {
  assert(pdg1 != nullptr);
  assert(pdg2 != nullptr);

  /*
   * Set the prefix string for the output.
   */
  std::string errorPrefix{ "PDG: Comparing two PDGs: " };
  if (verbose >= PDGVerbosity::Maximal) {
    errs() << errorPrefix << "Start\n";
  }

  /*
   * Code to invoke for missing dependences.
   */
  auto printErrorPDG1 = [&errorPrefix](DGEdge<Value> *d) {
    errs()
        << errorPrefix
        << "  PDG2 does not have the following dependence that exists in PDG1:\n";
    errs() << errorPrefix << "    From: " << *d->getOutgoingT() << "\n";
    errs() << errorPrefix << "    To: " << *d->getIncomingT() << "\n";
    return;
  };
  auto printErrorPDG2 = [&errorPrefix](DGEdge<Value> *d) {
    errs()
        << errorPrefix
        << "  PDG1 does not have the following dependence that exists in PDG2:\n";
    errs() << errorPrefix << "    From: " << *d->getOutgoingT() << "\n";
    errs() << errorPrefix << "    To: " << *d->getIncomingT() << "\n";
    return;
  };

  /*
   * Check the number of dependences are the same between the two PDGs.
   */
  if (pdg1->getNumberOfDependencesBetweenInstructions()
      != pdg2->getNumberOfDependencesBetweenInstructions()) {
    errs() << errorPrefix << "Number of PDG edges are not the same\n";
    errs() << errorPrefix << "  "
           << pdg1->getNumberOfDependencesBetweenInstructions() << "\n";
    errs() << errorPrefix << "  "
           << pdg2->getNumberOfDependencesBetweenInstructions() << "\n";
    this->compareEdges(pdg1, pdg2, printErrorPDG1);
    this->compareEdges(pdg2, pdg1, printErrorPDG2);
    return false;
  }

  /*
   * Check all dependences.
   */
  auto match = this->compareEdges(pdg1, pdg2, printErrorPDG1);

  return match;
}
