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
using namespace llvm::noelle;

namespace llvm::noelle {

  /*
   * Program Dependence Graph.
   */
  class PDG : public DG<Value> {
    public:

      /*
       * Constructor: 
       * Add all instructions included in the module M as nodes to the PDG.
       */
      PDG (Module &M) ;

      /*
       * Constructor: 
       * Add all instructions included in the function F as nodes to the PDG.
       */
      PDG (Function &F) ;

      /*
       * Constructor: 
       * Add all instructions included in the loop only.
       */
      PDG (Loop *loop) ;

      /*
       * Constructor: 
       * Add only the instructions given as parameter.
       */
      PDG (std::vector<Value *> &values) ;

      /*
       * Constructor: 
       * Empty graph.
       */
      PDG ();

      /*
       * Return the number of instructions included in the PDG.
       */
      int64_t getNumberOfInstructionsIncluded (void) const ;

      /*
       * Return the number of dependences of the PDG including dependences that connect instructions outside the PDG.
       */
      int64_t getNumberOfDependencesBetweenInstructions (void) const ;

      /*
       * Iterator: iterate over the instructions that depend on @param fromValue until @functionToInvokePerDependence returns true or there is no other dependence to iterate.
       *
       * This means there is an edge from @param fromValue to @param toValue of the type specified by the other parameters.
       * For each of this edge, the function @param functionToInvokePerDependence is invoked.
       *
       * This function returns true if the iteration ends earlier. 
       * It returns false otherwise.
       */
      bool iterateOverDependencesFrom (
        Value *fromValue, 
        bool includeControlDependences,
        bool includeMemoryDataDependences,
        bool includeRegisterDataDependences,
        std::function<bool (Value *to, DGEdge<Value> *dependence)> functionToInvokePerDependence
        );

      /*
       * Iterator: iterate over the instructions that @param toValue depends from until @functionToInvokePerDependence returns true or there is no other dependence to iterate.
       *
       * This means there is an edge from @param fromValue to @param toValue of the type specified by the other parameters.
       * For each of this edge, the function @param functionToInvokePerDependence is invoked.
       *
       * This function returns true if the iteration ends earlier. 
       * It returns false otherwise.
       */
      bool iterateOverDependencesTo (
        Value *toValue, 
        bool includeControlDependences,
        bool includeMemoryDataDependences,
        bool includeRegisterDataDependences,
        std::function<bool (Value *fromValue, DGEdge<Value> *dependence)> functionToInvokePerDependence
        );

      /*
       * Add the edge from "from" to "to" to the PDG.
       */
      DGEdge<Value> * addEdge (
        Value *from, 
        Value *to
        );

      /*
       * Creating Program Dependence Subgraphs
       */
      PDG * createFunctionSubgraph (Function &F);
      PDG * createLoopsSubgraph (Loop *loop);

      PDG * createSubgraphFromValues (std::vector<Value *> &valueList, bool linkToExternal);
      PDG * createSubgraphFromValues (
        std::vector<Value *> &valueList,
        bool linkToExternal,
        std::unordered_set<DGEdge<Value> *> edgesToIgnore
      );

      /*
       * Destructor
       */
      ~PDG() ;
      
    protected:

      void addNodesOf (Function &F);

      void setEntryPointAt (Function &F);

      void copyEdgesInto (PDG *newPDG, bool linkToExternal);

      void copyEdgesInto (PDG *newPDG, bool linkToExternal, std::unordered_set<DGEdge<Value> *> const & edgesToIgnore);
  };

}
