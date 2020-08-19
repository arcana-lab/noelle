/*
 * Copyright 2019 - 2020 Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"
#include "CallGraphNode.hpp"

namespace llvm::noelle {

  CallGraphFunctionNode::CallGraphFunctionNode (Function &func)
    : f{func}
    {

    return ;
  }
          
  Function * CallGraphFunctionNode::getFunction (void) const {
    auto ptr = &(this->f);

    return ptr;
  }

  void CallGraphFunctionNode::addOutgoingEdge (CallGraphFunctionFunctionEdge *edge){
    assert(edge->getCaller() == this);

    /*
     * Add the edge.
     */
    this->outgoingEdges.insert(edge);

    auto calleeNode = edge->getCallee();
    this->outgoingEdgesMap[calleeNode] = edge;

    return ;
  }

  void CallGraphFunctionNode::addIncomingEdge (CallGraphFunctionFunctionEdge *edge){
    assert(edge->getCallee() == this);

    /*
     * Add the edge.
     */
    this->incomingEdges.insert(edge);

    auto callerNode = edge->getCaller();
    this->incomingEdgesMap[callerNode] = edge;

    return ;
  }

  std::unordered_set<CallGraphFunctionFunctionEdge *> CallGraphFunctionNode::getIncomingEdges (void) const {
    return this->incomingEdges;
  }

  std::unordered_set<CallGraphFunctionFunctionEdge *> CallGraphFunctionNode::getOutgoingEdges (void) const {
    return this->outgoingEdges;
  }

  std::unordered_set<CallGraphFunctionFunctionEdge *> CallGraphFunctionNode::getEdges (void) const {
    std::unordered_set<CallGraphFunctionFunctionEdge *> edges{this->incomingEdges};
    edges.insert(this->outgoingEdges.begin(), this->outgoingEdges.end());

    return edges;
  }

  CallGraphFunctionFunctionEdge * CallGraphFunctionNode::getCallEdgeTo (CallGraphFunctionNode *callee) const {
    if (this->outgoingEdgesMap.find(callee) == this->outgoingEdgesMap.end()){
      return nullptr;
    }
    auto edge = this->outgoingEdgesMap.at(callee);

    return edge;
  }

  CallGraphFunctionFunctionEdge * CallGraphFunctionNode::getCallEdgeFrom (CallGraphFunctionNode *caller) const {
    if (this->incomingEdgesMap.find(caller) == this->incomingEdgesMap.end()){
      return nullptr;
    }
    auto edge = this->incomingEdgesMap.at(caller);

    return edge;
  }

  void CallGraphFunctionNode::print (void) {
    errs() << this->f.getName() << "\n";

    return ;
  }

  CallGraphInstructionNode::CallGraphInstructionNode (Instruction *i)
    :i{i}
    {
    return ;
  }

  Instruction * CallGraphInstructionNode::getInstruction (void) const {
    return i;
  }

  void CallGraphInstructionNode::print (void) {
    errs() << *this->i << "\n";

    return ;
  }

}
