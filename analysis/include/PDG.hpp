#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/GraphWriter.h"

using namespace std;
using namespace llvm;

namespace llvm {

  // Template PDG node to abstract node type
  template <class NodeT> 
  class PDGNodeBase {
    public:
      PDGNodeBase() { theNode = NULL; }
      PDGNodeBase(NodeT *node) { theNode = node; }

      typename std::vector<PDGNodeBase *>::iterator begin_nodes() { return outgoingNodes.begin(); }
      typename std::vector<PDGNodeBase *>::iterator end_nodes() { return outgoingNodes.end(); }

      /*
      Define iterators for outgoing and incoming edges
      
      using iterator = typename std::vector<PDGNodeBase *>::iterator;
      using const_iterator = typename std::vector<PDGNodeBase *>::const_iterator;

      iterator begin() { return children.begin(); }
      iterator end() { return children.end(); }
      const_iterator begin() const { return children.begin(); }
      const_iterator end() const { return children.end(); }
      */

      NodeT *getNode() const { return theNode; }

      std::string toString() { return "node"; }

      void addNode(PDGNodeBase *base) {
        outgoingNodes.push_back(base);
      }

    private:
      NodeT *theNode;
      std::vector<PDGNodeBase *> outgoingNodes;
  };

  template <>
  inline std::string PDGNodeBase<Instruction>::toString() {
    if (!theNode) {
      return "Empty node\n";
    }
    std::string str;
    raw_string_ostream ros(str);
    theNode->print(ros);
    return str;
  }

  /*
   * Program Dependence Graph.
   */
  class PDG {
    public:
      PDG(Module &M) ;
      
      typedef vector<PDGNodeBase<Instruction> *>::iterator nodes_iterator;
      typedef vector<PDGNodeBase<Instruction> *>::const_iterator nodes_const_iterator;

      /*
       * Iterators.
       */
      nodes_iterator begin_nodes() { allNodes.begin(); }
      nodes_iterator end_nodes() { allNodes.end(); }
      nodes_const_iterator begin_nodes() const { allNodes.begin(); }
      nodes_const_iterator end_nodes() const { allNodes.end(); }
      //TODO: Add edge iterator

      PDGNodeBase<Instruction> *getEntryNode() {
        return entryNode;
      }

    private:
      std::vector<PDGNodeBase<Instruction> *> allNodes;
      PDGNodeBase<Instruction> *entryNode;
      std::map<Instruction *, PDGNodeBase<Instruction> *> instructionNodes;

      void constructNodes (Module &M);
      void constructEdges (Module &M) ;
  };
}
