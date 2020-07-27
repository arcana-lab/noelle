/*
 * Copyright 2019 - 2020 Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"
#include "CallGraph.hpp"

using namespace llvm;
using namespace noelle;

noelle::CallGraph::CallGraph (Module &M, PTACallGraph *callGraph)
  : m{M}
  {

  /*
   * Create the function nodes.
   */
  for (auto &F: M){

    /*
     * Create a node for the current function.
     */
    auto newNode = new CallGraphFunctionNode(F);
    this->functions[&F] = newNode;
  }

  /*
   * Create the edges.
   */
  for (auto &F : M){

    /*
     * Fetch the node of the current function.
     */
    auto fromNode = this->functions[&F];

    /*
     * Add the edges from this function.
     */
    for (auto &inst : instructions(&F)){

      /*
       * Handle call instructions.
       */
      if (auto callInst = dyn_cast<CallInst>(&inst)){
        this->handleCallInstruction(fromNode, callInst);
        continue ;
      }

      /*
       * Handle invoke instructions.
       */
      if (auto callInst = dyn_cast<InvokeInst>(&inst)){
        this->handleCallInstruction(fromNode, callInst);
        continue ;
      }
    }
  }

  return ;
}

std::unordered_set<CallGraphFunctionNode *> noelle::CallGraph::getFunctionNodes (void) const {
  std::unordered_set<CallGraphFunctionNode *> s;

  for (auto pair : this->functions){
    s.insert(pair.second);
  }

  return s;
}

std::unordered_set<CallGraphEdge *> noelle::CallGraph::getEdges (void) const {
  return this->edges;
}

CallGraphFunctionNode * noelle::CallGraph::getFunctionNode (Function *f) const {
  if (this->functions.find(f) == this->functions.end()){
    return nullptr;
  }
  auto n = this->functions.at(f);

  return n;
}

void noelle::CallGraph::handleCallInstruction (CallGraphFunctionNode *fromNode, CallBase *callInst){

  /*
   * Fetch the callee.
   */
  auto callee = callInst->getCalledFunction();
  if (callee != nullptr){

    /*
     * Fetch the callee node.
     */
    assert(this->functions.find(callee) != this->functions.end());
    auto toNode = this->functions.at(callee);

    /*
     * Create the sub-edge.
     */
    auto instNode = this->instructionNodes[callInst];
    if (instNode == nullptr){
      instNode = new CallGraphInstructionNode(callInst);
      this->instructionNodes[callInst] = instNode;
    }
    auto subEdge = new CallGraphInstructionFunctionEdge(instNode, toNode, true);

    /*
     * Check if the edge already exists.
     */
    auto existingEdge = fromNode->getCallEdgeTo(toNode);
    if (existingEdge == nullptr){

      /*
       * The edge from @fromNode to @toNode doesn't exist yet.
       *
       * Create a new edge.
       */
      auto newEdge = new CallGraphFunctionFunctionEdge(fromNode, toNode, true);
      this->edges.insert(newEdge);

      /*
       * Add the new edge.
       */
      fromNode->addOutgoingEdge(newEdge);
      toNode->addIncomingEdge(newEdge);

      /*
       * Add the sub-edge.
       */
      newEdge->addSubEdge(subEdge);

      return ;
    }

    /*
     * The edge from @fromNode to @toNode already exists.
     *
     * Change its flag to must.
     */
    existingEdge->setMust();

      /*
     * Add the sub-edge.
     */
    existingEdge->addSubEdge(subEdge);

    return ;
  }

  /*
   * The callee is unknown.
   */
  //TODO

  return ;
}
        
SCCCAG * noelle::CallGraph::getSCCCAG (void) {

  /*
   * Check if we have already computed it.
   */
  if (this->scccag != nullptr){
    return this->scccag;
  }

  /*
   * Compute the SCCCAG.
   */
  this->scccag = new SCCCAG(this);

  return this->scccag;
}

bool noelle::CallGraph::doesItBelongToASCC (Function *f) {

  /*
   * Fetch the SCCCAG.
   */
  auto localAG = this->getSCCCAG();

  /*
   * Fetch the SCCCAG node of @f.
   */
  auto callGraphNode = this->getFunctionNode(f);
  assert(callGraphNode != nullptr);
  auto localAGNode = localAG->getNode(callGraphNode);
  assert(localAGNode != nullptr);

  /*
   * Check if the node belongs to an SCC.
   */
  if (localAGNode->isAnSCC()){
    return true;
  }

  /*
   * The node doesn't belong to an SCC.
   */
  return false;
}
