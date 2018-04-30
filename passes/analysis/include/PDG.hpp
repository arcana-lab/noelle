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
      PDG() ;
      ~PDG() ;

      /*
       * Add all instructions included in the module M as nodes to the PDG.
       */
      void addNodes(Module &M);

      /*
       * Add the edge from "from" to "to" to the PDG.
       */
      DGEdge<Value> * addEdge (Value *from, Value *to);

      /*
       * Creating Program Dependence Subgraphs
       */
      PDG *createFunctionSubgraph(Function &F);
      PDG *createLoopsSubgraph(LoopInfo &LI);
      PDG *createSubgraphFromValues(std::vector<Value *> &valueList, bool linkToExternal);
      
    private:
      void copyEdgesInto(PDG *newPDG, bool linkToExternal = true);
  };
}

