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
#include "CallGraphEdge.hpp"
#include "SCCCAG.hpp"

#include "MemoryModel/PointerAnalysis.h"
#include "Util/PTACallGraph.h"
#include "MSSA/MemSSA.h"

namespace llvm::noelle {
  class SCCCAG;

  /*
   * Call graph.
   */
  class CallGraph {
    public:
      CallGraph (Module &M, PTACallGraph *callGraph);

      std::unordered_set<CallGraphFunctionNode *> getFunctionNodes (void) const ;

      std::unordered_set<CallGraphEdge *> getEdges (void) const ;

      CallGraphFunctionNode * getEntryNode (void) const ;

      CallGraphFunctionNode * getFunctionNode (Function *f) const ;

      std::unordered_map<Function *, CallGraph *> getIslands (void) const ;

      SCCCAG * getSCCCAG (void) ;

      bool doesItBelongToASCC (Function *f) ;

    private:
      Module &m;
      std::unordered_map<Function *, CallGraphFunctionNode *> functions;
      std::unordered_map<Instruction *, CallGraphInstructionNode *> instructionNodes;
      std::unordered_set<CallGraphEdge *> edges;
      SCCCAG *scccag;

      CallGraph (Module &M);

      void handleCallInstruction (CallGraphFunctionNode *fromNode, CallBase *callInst, PTACallGraph *callGraph);

      CallGraphFunctionFunctionEdge * fetchOrCreateEdge (CallGraphFunctionNode *fromNode, CallBase *callInst, Function & callee, bool isMust);

      void identifyCallGraphIslandsByCallInstructions (std::unordered_map<Function *, CallGraph *> &islands) const ;

      void mergeCallGraphIslandsForEscapedFunctions (std::unordered_map<Function *, CallGraph *> &islands) const ;

  };

}
