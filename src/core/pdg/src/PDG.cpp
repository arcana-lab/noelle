/*
 * Copyright 2016 - 2024  Angelo Matni, Simone Campanoni
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
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include <set>
#include <queue>

#include "arcana/noelle/core/Utils.hpp"
#include "arcana/noelle/core/PDG.hpp"

namespace arcana::noelle {

PDG::PDG(Module &M) {

  /*
   * Create a node per instruction and function argument
   */
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    addNodesOf(F);
  }

  /*
   * Set the entry node: the first instruction of the function "main"
   */
  auto mainF = M.getFunction("main");
  assert(mainF != nullptr);

  this->setEntryPointAt(*mainF);
  for (auto edge : this->getEdges()) {
    assert(!edge->isLoopCarriedDependence() && "Flag was already set");
  }

  return;
}

PDG::PDG(Function &F) {
  addNodesOf(F);
  setEntryPointAt(F);

  return;
}

PDG::PDG(Loop *loop) {

  /*
   * Create a node per instruction within loops of LI only
   */
  for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi) {
    for (auto &I : **bbi) {
      this->addNode(cast<Value>(&I), /*inclusion=*/true);
    }
  }

  /*
   * Set the entry node: the first instruction of one of the top level loops
   * (See include/llvm/Analysis/LoopInfo.h:653)
   */
  auto bbBegin = *(loop->block_begin());
  this->entryNode = this->internalNodeMap[&*(bbBegin->begin())];
  assert(this->entryNode != nullptr);

  return;
}

PDG::PDG(std::vector<Value *> &values) {
  for (auto &V : values) {
    this->addNode(V, /*inclusion=*/true);
  }

  this->entryNode = this->internalNodeMap[*(values.begin())];
  assert(this->entryNode != nullptr);

  return;
}

void PDG::addNodesOf(Function &F) {
  for (auto &arg : F.args()) {
    addNode(cast<Value>(&arg), /*inclusion=*/true);
  }

  for (auto &B : F) {
    for (auto &I : B) {
      addNode(cast<Value>(&I), /*inclusion=*/true);
    }
  }
}

void PDG::setEntryPointAt(Function &F) {
  auto entryInstr = &*(F.begin()->begin());
  entryNode = internalNodeMap[entryInstr];
  assert(entryNode != nullptr);
}

PDG *PDG::createFunctionSubgraph(Function &F) {

  /*
   * Check if the function has a body.
   */
  if (F.empty())
    return nullptr;

  /*
   * Create the sub-PDG.
   */
  auto functionPDG = new PDG(F);

  /*
   * Recreate all edges connected to internal nodes of function
   */
  copyEdgesInto(functionPDG, /*linkToExternal=*/true);
  for (auto edge : functionPDG->getEdges()) {
    assert(!edge->isLoopCarriedDependence() && "Flag was already set");
  }
  return functionPDG;
}

PDG *PDG::createLoopsSubgraph(Loop *loop) {

  /*
   * Create a node per instruction within loops of LI only
   */
  auto loopsPDG = new PDG(loop);

  /*
   * Recreate all edges connected to internal nodes of loop
   */
  copyEdgesInto(loopsPDG, /*linkToExternal=*/true);

  return loopsPDG;
}

PDG *PDG::createSubgraphFromValues(std::vector<Value *> &valueList,
                                   bool linkToExternal) {
  return createSubgraphFromValues(valueList, linkToExternal, {});
}

PDG *PDG::createSubgraphFromValues(
    std::vector<Value *> &valueList,
    bool linkToExternal,
    std::unordered_set<DGEdge<Value, Value> *> edgesToIgnore) {
  if (valueList.empty())
    return nullptr;
  auto newPDG = new PDG(valueList);

  copyEdgesInto(newPDG, linkToExternal, edgesToIgnore);

  return newPDG;
}

void PDG::copyEdgesInto(PDG *newPDG, bool linkToExternal) {
  this->copyEdgesInto(newPDG, linkToExternal, {});

  return;
}

void PDG::copyEdgesInto(
    PDG *newPDG,
    bool linkToExternal,
    std::unordered_set<DGEdge<Value, Value> *> const &edgesToIgnore) {
  for (auto *oldEdge : allEdges) {
    if (edgesToIgnore.find(oldEdge) != edgesToIgnore.end()) {
      continue;
    }

    auto fromT = oldEdge->getSrc();
    auto toT = oldEdge->getDst();

    /*
     * Check whether edge belongs to nodes within function F
     */
    auto fromInclusion = newPDG->isInternal(fromT);
    auto toInclusion = newPDG->isInternal(toT);
    if (!fromInclusion && !toInclusion) {
      continue;
    }
    if (!linkToExternal && (!fromInclusion || !toInclusion)) {
      continue;
    }

    /*
     * Create appropriate external nodes and associate edge to them
     */
    newPDG->fetchOrAddNode(fromT, fromInclusion);
    newPDG->fetchOrAddNode(toT, toInclusion);

    /*
     * Copy edge to match properties (mem/var, must/may, RAW/WAW/WAR/control)
     */
    newPDG->copyAddEdge(*oldEdge);
  }

  return;
}

uint64_t PDG::getNumberOfInstructionsIncluded(void) const {
  return this->numInternalNodes();
}

uint64_t PDG::getNumberOfDependencesBetweenInstructions(void) const {
  return this->numEdges();
}

bool PDG::iterateOverDependencesFrom(
    Value *from,
    bool includeControlDependences,
    bool includeMemoryDataDependences,
    bool includeRegisterDataDependences,
    std::function<bool(Value *to, DGEdge<Value, Value> *dependence)>
        functionToInvokePerDependence) {

  /*
   * Fetch the node in the PDG.
   */
  auto pdgNode = this->fetchNode(from);
  if (pdgNode == nullptr) {
    return false;
  }

  /*
   * Iterate over the edges of the node.
   */
  for (auto edgeIt = pdgNode->begin_outgoing_edges();
       edgeIt != pdgNode->end_outgoing_edges();
       ++edgeIt) {

    /*
     * Fetch the destination value.
     */
    auto edge = *edgeIt;
    auto destValue = edge->getDst();

    /*
     * Check if this is a control dependence.
     */
    if (includeControlDependences
        && isa<ControlDependence<Value, Value>>(edge)) {
      if (functionToInvokePerDependence(destValue, edge)) {
        return true;
      }
      continue;
    }

    /*
     * Check if this is a memory dependence.
     */
    if (includeMemoryDataDependences
        && isa<MemoryDependence<Value, Value>>(edge)) {
      if (functionToInvokePerDependence(destValue, edge)) {
        return true;
      }
      continue;
    }

    /*
     * Check if this is a register dependence.
     */
    if (includeRegisterDataDependences
        && isa<VariableDependence<Value, Value>>(edge)) {
      if (functionToInvokePerDependence(destValue, edge)) {
        return true;
      }
      continue;
    }
  }

  return false;
}

bool PDG::iterateOverDependencesTo(
    Value *toValue,
    bool includeControlDependences,
    bool includeMemoryDataDependences,
    bool includeRegisterDataDependences,
    std::function<bool(Value *fromValue, DGEdge<Value, Value> *dependence)>
        functionToInvokePerDependence) {

  /*
   * Fetch the node in the PDG.
   */
  auto pdgNode = this->fetchNode(toValue);
  if (pdgNode == nullptr) {
    return false;
  }

  /*
   * Iterate over the edges of the node.
   */
  for (auto edgeIt = pdgNode->begin_incoming_edges();
       edgeIt != pdgNode->end_incoming_edges();
       ++edgeIt) {

    /*
     * Fetch the destination value.
     */
    auto edge = *edgeIt;
    auto srcValue = edge->getSrc();

    /*
     * Check if this is a control dependence.
     */
    if (includeControlDependences
        && isa<ControlDependence<Value, Value>>(edge)) {
      if (functionToInvokePerDependence(srcValue, edge)) {
        return true;
      }
      continue;
    }

    /*
     * Check if this is a memory dependence.
     */
    if (includeMemoryDataDependences
        && isa<MemoryDependence<Value, Value>>(edge)) {
      if (functionToInvokePerDependence(srcValue, edge)) {
        return true;
      }
      continue;
    }

    /*
     * Check if this is a register dependence.
     */
    if (includeRegisterDataDependences
        && isa<VariableDependence<Value, Value>>(edge)) {
      if (functionToInvokePerDependence(srcValue, edge)) {
        return true;
      }
      continue;
    }
  }

  return false;
}

std::vector<Value *> PDG::getSortedValues(void) {
  std::unordered_set<Value *> s;

  /*
   * Fetch all nodes.
   */
  auto nodes = this->getNodes();

  /*
   * Fetch all values stored in nodes.
   */
  for (auto node : nodes) {
    auto v = node->getT();
    s.insert(v);
  }

  /*
   * Create a sorted set of values.
   */
  auto sortedValues = Utils::sort(s);

  return sortedValues;
}

std::vector<DGEdge<Value, Value> *> PDG::getSortedDependences(void) {

  /*
   * Sort the edges.
   */
  auto v = DG<Value>::sortDependences(this->allEdges);

  return v;
}

std::unordered_set<DGEdge<Value, Value> *> PDG::getDependences(Value *from,
                                                               Value *to) {

  /*
   * Fetch the nodes.
   */
  auto srcNode = this->fetchNode(from);
  auto dstNode = this->fetchNode(to);
  if (!srcNode || !dstNode) {
    return {};
  }

  /*
   * Fetch the dependences.
   */
  auto edgeSet = this->fetchEdges(srcNode, dstNode);

  return edgeSet;
}

PDG *PDG::clone(bool includeExternalNodes) {

  /*
   * Collect nodes to include in the clone.
   */
  std::vector<Value *> currentNodes;
  for (auto internalNode : this->internalNodePairs()) {
    currentNodes.push_back(internalNode.first);
  }
  if (includeExternalNodes) {
    for (auto externalNode : this->externalNodePairs()) {
      currentNodes.push_back(externalNode.first);
    }
  }

  /*
   * Clone the DG.
   */
  auto cloneDG =
      this->createSubgraphFromValues(currentNodes, includeExternalNodes);

  return cloneDG;
}

PDG::~PDG() {
  for (auto *edge : allEdges)
    if (edge)
      delete edge;
  for (auto *node : allNodes)
    if (node)
      delete node;
}

} // namespace arcana::noelle
