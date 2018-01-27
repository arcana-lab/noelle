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
template <class NodeT> class PDGNodeBase {
  NodeT *theNode;
  std::vector<PDGNodeBase *> outgoingNodes;

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
};

template <>
std::string PDGNodeBase<Instruction>::toString() {
  if (!theNode) {
    return "Root (empty) node\n";
  }
  std::string str;
  raw_string_ostream ros(str);
  theNode->print(ros);
  return str;
}

// PDG using Instructions as nodes
class PDG {
 private:
  std::vector<PDGNodeBase<Instruction> *> allNodes;
  PDGNodeBase<Instruction> *entryNode;
  std::map<Instruction *, PDGNodeBase<Instruction> *> instructionNodes;

 public:
  PDG(Module &M) {
    constructNodes(M);
    constructEdges(M);
  }
  
  typedef vector<PDGNodeBase<Instruction> *>::iterator nodes_iterator;
  typedef vector<PDGNodeBase<Instruction> *>::const_iterator nodes_const_iterator;

  nodes_iterator begin_nodes() { allNodes.begin(); }
  nodes_iterator end_nodes() { allNodes.end(); }
  nodes_const_iterator begin_nodes() const { allNodes.begin(); }
  nodes_const_iterator end_nodes() const { allNodes.end(); }

  // Add edge iterator

  void constructNodes(Module &M) {
    for (auto &F : M) {
      for (auto &B : F) {
        for (auto &I : B) {
          PDGNodeBase<Instruction> *node = new PDGNodeBase<Instruction>(&I);
          allNodes.push_back(node);
          instructionNodes[&I] = node;
        }
      }
    }
    auto mainF = M.getFunction("main");
    if (mainF == nullptr) {
      errs() << "ERROR: Main function not found\n";
      abort();
    }
    auto entryInstr = &*(mainF->begin()->begin());
    entryNode = instructionNodes[entryInstr];
    assert(entryNode != nullptr);
  }

  void constructEdges(Module &M) {
    for (auto &F : M) {
      for (auto &B : F) {
        for (auto &I : B) {
          PDGNodeBase<Instruction> *iNode = instructionNodes[&I];
          if (I.getNumUses() == 0)
            continue;
          for (auto& U : I.uses()) {
            auto user = U.getUser();
            if (auto userInst = dyn_cast<Instruction>(user)){
              iNode->addNode(instructionNodes[userInst]);
            }
          }
        }
      }
    }
  }

  PDGNodeBase<Instruction> *getEntryNode() {
    return entryNode;
  }
};

}