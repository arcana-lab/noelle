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

#include "MemoryModel/PointerAnalysis.h"
#include "Util/PTACallGraph.h"
#include "MSSA/MemSSA.h"

namespace llvm {

  namespace noelle {

    /*
     * Call graph.
     */
    class CallGraph {
      public:
        CallGraph (Module &M, PTACallGraph *callGraph);

        std::unordered_set<CallGraphFunctionNode *> getFunctionNodes (void) const ;

        std::unordered_set<CallGraphEdge *> getEdges (void) const ;

        CallGraphFunctionNode * getFunctionNode (Function *f) const ;

        bool doesItBelongToASCC (Function *f) ;

      private:
        Module &m;
        std::unordered_map<Function *, CallGraphFunctionNode *> functions;
        std::unordered_map<Instruction *, CallGraphInstructionNode *> instructionNodes;
        std::unordered_set<CallGraphEdge *> edges;

        void handleCallInstruction (CallGraphFunctionNode *fromNode, CallBase *callInst);
    };

  }
}
