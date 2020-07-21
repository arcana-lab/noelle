/*
 * Copyright 2016 - 2020  Angelo Matni, Yian Su, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"

#include "PDGPrinter.hpp"
#include "PDGAnalysis.hpp"

using namespace llvm;

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

bool PDGAnalysis::compareEdges(PDG *pdg1, PDG *pdg2) {
  if (verbose >= PDGVerbosity::Maximal) {
    errs() << "Compare PDG Edges\n";
  }

  if (pdg1->numEdges() != pdg2->numEdges()) {
    std::string errorPrefix{"PDG: Comparing two PDGs: "};
    errs() << errorPrefix << "Number of pdg edges are not the same\n";
    errs() << errorPrefix << "  " << pdg1->numEdges() << "\n";
    errs() << errorPrefix << "  " << pdg2->numEdges() << "\n";
    return false;
  }

  for (auto &edge1 : pdg1->getEdges()) {
    auto outgoingNode = pdg2->fetchNode(edge1->getOutgoingT());
    auto incomingNode = pdg2->fetchNode(edge1->getIncomingT());
    if (!outgoingNode || !incomingNode) {
      return false;
    }
    auto edgeSet = pdg2->fetchEdges(outgoingNode, incomingNode);
    if (edgeSet.empty()) {
      return false;
    }

    auto match = false;
    for (auto &edge2 : edgeSet) {
      if (edge1->isMemoryDependence() == edge2->isMemoryDependence() &&
          edge1->isMustDependence() == edge2->isMustDependence() &&
          edge1->isControlDependence() == edge2->isControlDependence() &&
          edge1->isLoopCarriedDependence() == edge2->isLoopCarriedDependence() &&
          edge1->isRemovableDependence() == edge2->isRemovableDependence() &&
          edge1->dataDependenceType() == edge2->dataDependenceType()) {
        match = true;
        break;
      }
    }
    if (!match) {
      return false;
    }
  }

  return true;
}

