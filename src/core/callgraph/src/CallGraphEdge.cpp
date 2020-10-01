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
#include "CallGraphEdge.hpp"

namespace llvm::noelle{

  void CallGraphEdge::setMust (void) {
    this->isMust = true;

    return ;
  }

  void CallGraphEdge::unsetMust (void){
    this->isMust = false;

    return ;
  }

  bool CallGraphEdge::isAMustCall (void) const {
    return this->isMust;
  }

  CallGraphFunctionNode * CallGraphEdge::getCallee (void) const {
    return this->callee;
  }

  CallGraphFunctionFunctionEdge::CallGraphFunctionFunctionEdge (CallGraphFunctionNode *caller, CallGraphFunctionNode *callee, bool isMust)
    : caller{caller}
    {
    this->isMust = isMust;
    this->callee = callee;

    return ;
  }

  CallGraphFunctionNode * CallGraphFunctionFunctionEdge::getCaller (void) const {
    return this->caller;
  }

  void CallGraphFunctionFunctionEdge::print (void) {
    errs() << "TODO\n";

    return ;
  }

  void CallGraphFunctionFunctionEdge::addSubEdge (CallGraphInstructionFunctionEdge *subEdge){

    /*
     * Fetch the caller.
     */
    auto instNode = subEdge->getCaller();
    auto inst = instNode->getInstruction();

    /*
     * Add the sub-edge.
     */
    this->subEdges.insert(subEdge);
    this->subEdgesMap[inst] = subEdge;

    return ;
  }

  std::unordered_set<CallGraphInstructionFunctionEdge *> CallGraphFunctionFunctionEdge::getSubEdges (void) const {
    return this->subEdges;
  }
          
  CallGraphInstructionFunctionEdge::CallGraphInstructionFunctionEdge (CallGraphInstructionNode *caller, CallGraphFunctionNode *callee, bool isMust){
    this->caller = caller;
    this->callee = callee;
    this->isMust = isMust;

    return ;
  }

  CallGraphInstructionNode * CallGraphInstructionFunctionEdge::getCaller (void) const {
    return this->caller;
  }

  void CallGraphInstructionFunctionEdge::print (void) {
    errs() << "TODO\n";

    return ;
  }

}
