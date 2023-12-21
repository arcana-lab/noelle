/*
 * Copyright 2016 - 2023  Yian Su, Simone Campanoni
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
#include "noelle/core/TalkDown.hpp"
#include "noelle/core/PDGPrinter.hpp"
#include "noelle/core/PDGAnalysis.hpp"

namespace arcana::noelle {

MDNode *PDGAnalysis::getEdgeMetadata(
    DGEdge<Value, Value> *edge,
    LLVMContext &C,
    unordered_map<Value *, MDNode *> &nodeIDMap) {
  assert(edge != nullptr);
  Metadata *edgeM[] = {
    nodeIDMap[edge->getSrc()],
    nodeIDMap[edge->getDst()],
    MDNode::get(
        C,
        MDString::get(C, edge->isMemoryDependence() ? "true" : "false")),
    MDNode::get(C,
                MDString::get(C, edge->isMustDependence() ? "true" : "false")),
    MDNode::get(C, MDString::get(C, edge->dataDepToString())),
    MDNode::get(
        C,
        MDString::get(C, isa<ControlDependence<Value,Value>>(edge) ? "true" : "false")),
    MDNode::get(
        C,
        MDString::get(C, edge->isLoopCarriedDependence() ? "true" : "false")),
    getSubEdgesMetadata(edge, C, nodeIDMap)
  };

  return MDNode::get(C, edgeM);
}

MDNode *PDGAnalysis::getSubEdgesMetadata(
    DGEdge<Value, Value> *edge,
    LLVMContext &C,
    unordered_map<Value *, MDNode *> &nodeIDMap) {
  vector<Metadata *> subEdgesVec;

  for (auto &subEdge : edge->getSubEdges()) {
    Metadata *subEdgeM[] = {
      nodeIDMap[subEdge->getSrc()],
      nodeIDMap[subEdge->getDst()],
      MDNode::get(
          C,
          MDString::get(C, edge->isMemoryDependence() ? "true" : "false")),
      MDNode::get(
          C,
          MDString::get(C, edge->isMustDependence() ? "true" : "false")),
      MDNode::get(C, MDString::get(C, edge->dataDepToString())),
      MDNode::get(
          C,
          MDString::get(C, isa<ControlDependence<Value, Value>>(edge) ? "true" : "false")),
      MDNode::get(
          C,
          MDString::get(C, edge->isLoopCarriedDependence() ? "true" : "false")),
    };
    subEdgesVec.push_back(MDNode::get(C, subEdgeM));
  }

  return MDTuple::get(C, subEdgesVec);
}

bool PDGAnalysis::hasPDGAsMetadata(Module &M) {
  if (auto n = M.getNamedMetadata("noelle.module.pdg")) {
    if (auto m = dyn_cast<MDNode>(n->getOperand(0))) {
      if (cast<MDString>(m->getOperand(0))->getString() == "true") {
        return true;
      }
    }
  }

  return false;
}

PDG *PDGAnalysis::constructPDGFromMetadata(Module &M) {
  if (verbose >= PDGVerbosity::Maximal) {
    errs() << "PDGAnalysis: Construct PDG from Metadata\n";
  }

  /*
   * Create the PDG.
   */
  auto pdg = new PDG(M);

  /*
   * Fill up the PDG.
   */
  std::unordered_map<MDNode *, Value *> IDNodeMap;
  for (auto &F : M) {
    constructNodesFromMetadata(pdg, F, IDNodeMap);
    constructEdgesFromMetadata(pdg, F, IDNodeMap);
  }

  constructEdgesFromUseDefs(pdg);
  constructEdgesFromControl(pdg, M);

  return pdg;
}

void PDGAnalysis::constructNodesFromMetadata(
    PDG *pdg,
    Function &F,
    unordered_map<MDNode *, Value *> &IDNodeMap) {

  /*
   * Construct id to node map and add nodes of arguments to pdg
   */
  if (MDNode *argsM = F.getMetadata("noelle.pdg.args.id")) {
    for (auto &arg : F.args()) {
      if (MDNode *m = dyn_cast<MDNode>(argsM->getOperand(arg.getArgNo()))) {
        IDNodeMap[m] = &arg;
      }
    }
  }

  /*
   * Construct id to node map and add nodes of instructions to pdg
   */
  for (auto &B : F) {
    for (auto &I : B) {
      if (MDNode *m = I.getMetadata("noelle.pdg.inst.id")) {
        IDNodeMap[m] = &I;
      }
    }
  }

  return;
}

void PDGAnalysis::constructEdgesFromMetadata(
    PDG *pdg,
    Function &F,
    unordered_map<MDNode *, Value *> &IDNodeMap) {

  /*
   * Construct edges and set attributes
   */
  if (auto edgesM = F.getMetadata("noelle.pdg.edges")) {
    for (auto &operand : edgesM->operands()) {
      if (auto edgeM = dyn_cast<MDNode>(operand)) {
        auto edge = constructEdgeFromMetadata(pdg, edgeM, IDNodeMap);

        /*
         * Construct subEdges and set attributes
         */
        if (auto subEdgesM = dyn_cast<MDNode>(edgeM->getOperand(7))) {
          for (auto &subOperand : subEdgesM->operands()) {
            if (MDNode *subEdgeM = dyn_cast<MDNode>(subOperand)) {
              auto subEdge = constructEdgeFromMetadata(pdg, subEdgeM, IDNodeMap);
              edge->addSubEdge(subEdge);
            }
          }
        }

        /*
         * Add edge to pdg
         */
        pdg->copyAddEdge(*edge);

        /*
         * Free the memory.
         */
        delete edge;
      }
    }
  }

  return;
}

DGEdge<Value, Value> *PDGAnalysis::constructEdgeFromMetadata(
    PDG *pdg,
    MDNode *edgeM,
    unordered_map<MDNode *, Value *> &IDNodeMap) {
  DGEdge<Value, Value> *edge = nullptr;

  if (auto fromM = dyn_cast<MDNode>(edgeM->getOperand(0))) {
    if (auto toM = dyn_cast<MDNode>(edgeM->getOperand(1))) {
      Value *from = IDNodeMap[fromM];
      Value *to = IDNodeMap[toM];

      /*
       * Fetch the attributes.
       */
      auto isMemoryDependence = cast<MDString>(cast<MDNode>(edgeM->getOperand(2))->getOperand(0))->getString() == "true";
      auto isControl = cast<MDString>(cast<MDNode>(edgeM->getOperand(5))->getOperand(0))->getString() == "true";
      auto isLoopCarried = cast<MDString>(cast<MDNode>(edgeM->getOperand(6))->getOperand(0))->getString() == "true";

      /*
       * Allocate the dependence.
       */
      if (isControl){
        edge = new ControlDependence<Value, Value>(pdg->fetchNode(from), pdg->fetchNode(to));
      } else {
        auto k = DGEdge<Value, Value>::DependenceKind::VARIABLE_DEPENDENCE;
        if (isMemoryDependence){
          k = DGEdge<Value, Value>::DependenceKind::MEMORY_DEPENDENCE;
        }
        edge = new DataDependence<Value, Value>(k, pdg->fetchNode(from), pdg->fetchNode(to));
      }
      edge->setEdgeAttributes(
          isMemoryDependence,
          cast<MDString>(cast<MDNode>(edgeM->getOperand(3))->getOperand(0))
                  ->getString()
              == "true",
          cast<MDString>(cast<MDNode>(edgeM->getOperand(4))->getOperand(0))->getString().str(),
          isLoopCarried
          );
    }
  }

  return edge;
}

} // namespace arcana::noelle
