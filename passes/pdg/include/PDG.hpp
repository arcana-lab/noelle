/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/ADT/iterator_range.h"
#include <set>

#include "DGBase.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

  /*
   * Program Dependence Graph.
   */
  class PDG : public DG<Value> {
    public:

      /*
       * Constructor
       */
      PDG() ;

      /*
       * Add all instructions included in the module M as nodes to the PDG.
       */
      void populateNodesOf (Module &M);

      /*
       * Add all instructions included in the function F as nodes to the PDG.
       */
      void populateNodesOf (Function &F);

      /*
       * Add the edge from "from" to "to" to the PDG.
       */
      DGEdge<Value> * addEdge (Value *from, Value *to);

      /*
       * Creating Program Dependence Subgraphs
       */
      PDG * createFunctionSubgraph (Function &F);
      PDG * createLoopsSubgraph (Loop *loop);
      PDG * createSubgraphFromValues (std::vector<Value *> &valueList, bool linkToExternal);

      /*
       * Destructor
       */
      ~PDG() ;
      
    private:
      void addNodesOf (Function &F);
      void setEntryPointAt (Function &F);
      void copyEdgesInto (PDG *newPDG, bool linkToExternal = true);
  };

}
