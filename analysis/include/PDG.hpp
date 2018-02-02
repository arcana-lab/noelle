#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include <set>

#include "PDGBase.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

  /*
   * Alias information on each function used by the Program Dependence Graph
   */
  struct FunctionAliasInfo {
    FunctionAliasInfo(AAResults *a) { aa = a; }

    AAResults* aa;
    std::map<Instruction *, std::set<Instruction *>> mayAliases;
    std::map<Instruction *, std::set<Instruction *>> mustAliases;
  };

  /*
   * Program Dependence Graph.
   */
  class PDG {
    public:
      PDG(Module &M) ;
      
      typedef vector<PDGNodeBase<Instruction> *>::iterator nodes_iterator;
      typedef vector<PDGNodeBase<Instruction> *>::const_iterator nodes_const_iterator;

      typedef vector<PDGEdge *>::iterator edges_iterator;
      typedef vector<PDGEdge *>::const_iterator edges_const_iterator;

      /*
       * Iterators.
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

    private:
      std::vector<PDGNodeBase<Instruction> *> allNodes;
      std::vector<PDGEdge *> allEdges;
      PDGNodeBase<Instruction> *entryNode;
      std::map<Instruction *, PDGNodeBase<Instruction> *> instructionNodes;

      std::map<Function *, FunctionAliasInfo *> aliasInfo;

      void constructNodes (Module &M) ;
      void constructUseDefEdges (Module &M) ;
      void constructAliasingEdges (Module &M) ;
      void collectAliasPairs (Module &M) ;

  };
}
