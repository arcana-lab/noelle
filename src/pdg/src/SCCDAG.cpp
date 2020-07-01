/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <SystemHeaders.hpp>
#include "DGGraphTraits.hpp"
#include "SCCDAG.hpp"
#include "llvm/InitializePasses.h"

using namespace llvm ;

SCCDAG::SCCDAG(PDG *pdg) {

  /*
   * Create nodes of the SCCDAG.
   *
   * Iterate over all nodes in the PDG to calculate strongly connected components (see Tarjan's DFS algo).
   */
  std::set<DGNode<Value> *> visited;
  std::set<std::set<DGNode<Value> *>> sccs;
  DGNode<Value> *originalEntryNode = pdg->getEntryNode();

  for (auto nodeToVisit : pdg->getNodes()) {
    if (visited.find(nodeToVisit) != visited.end()) continue;

    pdg->setEntryNode(nodeToVisit);
    for (auto pdgI = scc_begin(pdg); pdgI != scc_end(pdg); ++pdgI) {

      /*
       * Identify a new SCC.
       */
      const std::vector<DGNode<Value> *> &sccNodes = *pdgI;
      if (visited.find(*sccNodes.begin()) != visited.end()) {
        continue;
      }

      /*
       * Add a new SCC to the SCCDAG.
       */
      std::set<DGNode<Value> *> nodes(sccNodes.begin(), sccNodes.end());
      visited.insert(nodes.begin(), nodes.end());
      auto scc = new SCC(nodes);
      auto isInternal = false;
      for (auto node : nodes) {
        isInternal |= pdg->isInternal(node->getT());
      }

      this->addNode(scc, /*inclusion=*/ isInternal);
    }
  }

  pdg->setEntryNode(originalEntryNode);

  /*
   * Create the map from a Value to an SCC included in the SCCDAG.
   */
  this->markValuesInSCC();

  /*
   * Create dependences between nodes of the SCCDAG.
   */
  this->markEdgesAndSubEdges();

  return ;
}
      
bool SCCDAG::doesItContain (Instruction *inst) const {

  /*
   * Fetch the SCC that contains the instruction given as input.
   */
  auto SCC = this->sccOfValue(inst);

  return SCC != nullptr;
}

void SCCDAG::markValuesInSCC (void) {

  /*
   * Maintain association of each SCC's node value to its SCC
   */
  this->valueToSCCNode.clear();
  for (auto sccNode : this->getNodes()) {
    auto scc = sccNode->getT();
    for (auto instPair : scc->internalNodePairs()) {
      this->valueToSCCNode[instPair.first] = sccNode;
    }
  }
}

void SCCDAG::markEdgesAndSubEdges (void) {

  /*
   * Add edges between SCCs by looking at each SCC's outgoing edges
   *
   * Iterate across SCCs.
   */
  std::set<DGEdge<SCC> *> clearedEdges;
  for (auto outgoingSCCNode : this->getNodes()){

    /*
     * Fetch the current SCC.
     */
    auto outgoingSCC = outgoingSCCNode->getT();

    /*
     * Check dependences that go outside the current SCC.
     */
    for (auto externalNodePair : outgoingSCC->externalNodePairs()) {
      auto incomingNode = externalNodePair.second;
      if (incomingNode->numIncomingEdges() == 0) continue;

      auto incomingSCCNode = this->valueToSCCNode[externalNodePair.first];
      auto incomingSCC = incomingSCCNode->getT();

      /*
       * Find or create unique edge between the two connected SCC
       */
      auto edgeSet = outgoingSCCNode->getEdgesToAndFromNode(incomingSCCNode);
      auto sccEdge = edgeSet.empty() ? this->addEdge(outgoingSCC, incomingSCC) : (*edgeSet.begin());

      /*
       * Clear out subedges if not already done once; add all currently existing subedges
       */
      if (clearedEdges.find(sccEdge) == clearedEdges.end())
      {
        sccEdge->clearSubEdges();
        clearedEdges.insert(sccEdge);
      }
      for (auto edge : incomingNode->getIncomingEdges()) sccEdge->addSubEdge(edge);
    }
  }
}

void SCCDAG::mergeSCCs(std::set<DGNode<SCC> *> &sccSet)
{
  if (sccSet.size() < 2) return;
  
  std::set<DGNode<Value> *> mergeNodes;
  for (auto sccNode : sccSet)
  {
    for (auto internalNodePair : sccNode->getT()->internalNodePairs())
    {
      mergeNodes.insert(internalNodePair.second);
    }
  }

  /*
   * Note: nodes are from 2 contexts; internal nodes will point to external nodes,
   *  some of whose values are in nodes in this list, and some of whose values are NOT in nodes in this list.
   *  However, SCC's constructor accounts for that context mismatch and properly copies edges WITHOUT
   *  duplicating any nodes or edges.
   */
  auto mergeSCC = new SCC(mergeNodes);

  /*
   * Add the new SCC and remove the old ones
   * Reassign values to the SCC they are now in
   * Recreate all edges from SCCs to the newly merged SCC
   */
  auto mergeSCCNode = this->addNode(mergeSCC, /*inclusion=*/ true);
  for (auto sccNode : sccSet) this->removeNode(sccNode);
  this->markValuesInSCC();
  this->markEdgesAndSubEdges();
}

SCC * SCCDAG::sccOfValue (Value *val) const {
  auto sccIter = valueToSCCNode.find(val);
  return sccIter == valueToSCCNode.end() ? nullptr : sccIter->second->getT();
}

int64_t SCCDAG::numberOfInstructions (void) {

  /*
   * Iterate over SCCs.
   */
  int64_t n = 0;
  for (auto SCCPair : this->internalNodePairs()){
    auto SCC = SCCPair.first;
    n += SCC->numberOfInstructions();
  }

  return n;
}

bool SCCDAG::iterateOverInstructions (std::function<bool (Instruction *)> funcToInvoke){

  /*
   * Iterate over SCC.
   */
  for (auto sccNodePair : this->internalNodePairs()){

    /*
     * Iterate over instructions contained in the SCC.
     */
    auto SCC = sccNodePair.first;
    if (SCC->iterateOverInstructions(funcToInvoke)){
      return true;
    }
  }

  return false ;
}

bool SCCDAG::iterateOverLiveInAndLiveOut (std::function<bool (Value *)> funcToInvoke){

  /*
   * Iterate over live-ins and live-outs of SCCs.
   *
   * A live-in/live-out is an SCC, which in the simplest case, it is a Value
   */
  for (auto sccNodePair : this->externalNodePairs()){

    /*
     * Iterate over internal nodes of the current SCC.
     */
    auto SCC = sccNodePair.first;
    if (SCC->iterateOverValues(funcToInvoke)){
      return true;
    }
  }

  return false ;
}

bool SCCDAG::iterateOverAllInstructions (std::function<bool (Instruction *)> funcToInvoke){

  /*
   * Iterate over SCC.
   */
  for (auto sccNode : this->getNodes()){

    /*
     * Iterate over instructions contained in the SCC.
     */
    auto SCC = sccNode->getT();
    if (SCC->iterateOverAllInstructions(funcToInvoke)){
      return true;
    }
  }

  return false ;
}

bool SCCDAG::iterateOverAllValues (std::function<bool (Value *)> funcToInvoke){

  /*
   * Iterate over SCC.
   */
  for (auto sccNode : this->getNodes()){

    /*
     * Iterate over instructions contained in the SCC.
     */
    auto SCC = sccNode->getT();
    if (SCC->iterateOverAllValues(funcToInvoke)){
      return true;
    }
  }

  return false ;
}

bool SCCDAG::iterateOverSCCs (std::function<bool (SCC *)> funcToInvoke){

  /*
   * Iterate over SCC.
   */
  for (auto sccNode : this->getNodes()){
    if (funcToInvoke(sccNode->getT())){
      return true;
    }
  }

  return false ;
}
      
std::unordered_set<SCC *> SCCDAG::getSCCs (void) {
  std::unordered_set<SCC *> s;
  for (auto sccNode : this->getNodes()){
    s.insert(sccNode->getT());
  }

  return s;
}

SCCDAG::~SCCDAG() {
  for (auto *edge : allEdges){
    if (edge) {
      delete edge;
    }
  }

  for (auto *node : allNodes){
    if (node) {
      auto scc = node->getT();
      delete scc;
      delete node;
    }
  }

  return ;
}
