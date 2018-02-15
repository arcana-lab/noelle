#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/ADT/iterator_range.h"
#include <set>

#include "PDGBase.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

  /*
   * Program Dependence Graph.
   */
  class PDG {
    public:
      PDG() ;
      ~PDG() ;

      typedef vector<PDGNodeBase<Instruction> *>::iterator nodes_iterator;
      typedef vector<PDGNodeBase<Instruction> *>::const_iterator nodes_const_iterator;

      typedef vector<PDGEdge *>::iterator edges_iterator;
      typedef vector<PDGEdge *>::const_iterator edges_const_iterator;

      /*
       * Node and Edge Iterators
       */
      nodes_iterator begin_nodes() { allNodes.begin(); }
      nodes_iterator end_nodes() { allNodes.end(); }
      nodes_const_iterator begin_nodes() const { allNodes.begin(); }
      nodes_const_iterator end_nodes() const { allNodes.end(); }

      nodes_iterator begin_edges() { allEdges.begin(); }
      nodes_iterator end_edges() { allEdges.end(); }
      nodes_const_iterator begin_edges() const { allEdges.begin(); }
      nodes_const_iterator end_edges() const { allEdges.end(); }

      PDGNodeBase<Instruction> *getEntryNode() {
        return entryNode;
      }

      PDGNodeBase<Instruction> *getNodeOf(Instruction *I) {
        return instructionNodes[I];
      }

      /*
       * Instruction Node Pair Iterator
       */
      iterator_range<typename std::map<Instruction *, PDGNodeBase<Instruction> *>::iterator>
      instructionNodePairs() {
        return make_range(instructionNodes.begin(), instructionNodes.end());
      }

      /*
       * Creating Nodes and Edges
       */
      void constructNodes(Module &M);
      PDGNodeBase<Instruction> *createNodeFrom(Instruction *I);
      PDGEdge *createEdgeFromTo(Instruction *from, Instruction *to);

      /*
       * Creating Program Dependence Subgraphs
       */
      PDG *createFunctionSubgraph(Function &F);
      PDG *createLoopsSubgraph(LoopInfo &LI);

    private:
      std::vector<PDGNodeBase<Instruction> *> allNodes;
      std::vector<PDGEdge *> allEdges;
      PDGNodeBase<Instruction> *entryNode;
      std::map<Instruction *, PDGNodeBase<Instruction> *> instructionNodes;
  };
}

