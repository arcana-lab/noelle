#pragma once

#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/GraphWriter.h"

// For df_iterator of nodes in graph
#include "llvm/ADT/DepthFirstIterator.h"

using namespace std;
using namespace llvm;

namespace llvm {

// Template PDG node to abstract node type
template <class NodeT> class PDGNodeBase {
  NodeT *theNode;
  std::vector<PDGNodeBase *> children;

 public:
  PDGNodeBase() { theNode = NULL; }
  PDGNodeBase(NodeT *node) { theNode = node; }

  using iterator = typename std::vector<PDGNodeBase *>::iterator;
  using const_iterator = typename std::vector<PDGNodeBase *>::const_iterator;

  iterator begin() { return children.begin(); }
  iterator end() { return children.end(); }
  const_iterator begin() const { return children.begin(); }
  const_iterator end() const { return children.end(); }

  NodeT *getNode() const { return theNode; }

  void addChild(PDGNodeBase *base) {
    children.push_back(base);
  }
};

using PDGNode = PDGNodeBase<Instruction>;

// Template PDG to be able to use abstract node types
template <typename NodeT> class PDGBase {
 protected:
  std::vector<PDGNodeBase<NodeT> *> roots;
  PDGNodeBase<NodeT> *rootNode;

 public:
  PDGBase();

  PDGNodeBase<NodeT> *getRootNode() {
    return rootNode;
  }
};

// PDG using Instructions as nodes
class PDG : public PDGBase<Instruction> {
 private:
  std::map<Instruction *, PDGNode *> instructionNodes;

 public:
  PDG();
  PDG(Module &M) {
    constructNodes(M);
    constructTree(M);
    attachEmptyRootNode();
  }

  void constructNodes(Module &M) {
    for (auto &F : M) {
      for (auto &B : F) {
        for (auto &I : B) {
          errs() << "Printing instruction\n";
          I.print(errs() << "Instruction: ");
          errs() << "\n";
          PDGNode *node = new PDGNode(&I);
          instructionNodes[&I] = node;
        }
      }
    }
  }

  void constructTree(Module &M) {
    for (auto &F : M) {
      for (auto &B : F) {
        for (auto &I : B) {
          PDGNode *iNode = instructionNodes[&I];
          if (I.getNumUses() == 0) {
            roots.push_back(iNode);
            continue;
          }
          for (auto& U : I.uses()) {
            Instruction* user = (Instruction*)(U.getUser());
            iNode->addChild(instructionNodes[user]);
          }
        }
      }
    }
  }

  void attachEmptyRootNode() {
    rootNode = new PDGNode();
    for (auto root : roots) {
      rootNode->addChild(root);
    }
  }
};

// Template PDG graph traits for use in graph node iteration
template <class Node, class ChildIterator> struct PDGGraphTraitsBase {
  using nodes_iterator = df_iterator<Node*, df_iterator_default_set<Node*>>;
  using NodeRef = Node *;
  using ChildIteratorType = ChildIterator;

  static NodeRef getEntryNode(NodeRef N) { return N; }
  static ChildIteratorType child_begin(NodeRef N) { return N->begin(); }
  static ChildIteratorType child_end(NodeRef N) { return N->end(); }

  static nodes_iterator nodes_begin(NodeRef N) {
    return df_begin(getEntryNode(N));
  }
  static nodes_iterator nodes_end(NodeRef N) { return df_end(getEntryNode(N)); }
};

template <> struct GraphTraits<PDGNode *>
  : public PDGGraphTraitsBase<PDGNode, PDGNode::iterator> {};

template <> struct GraphTraits<const PDGNode *>
  : public PDGGraphTraitsBase<const PDGNode, PDGNode::const_iterator> {};

template <> struct GraphTraits<PDG*> : public GraphTraits<PDGNode*> {
  static NodeRef getEntryNode(PDG *pdg) { return pdg->getRootNode(); }

  static nodes_iterator nodes_begin(PDG *pdg) {
    return df_begin(getEntryNode(pdg));
  }

  static nodes_iterator nodes_end(PDG *pdg) {
    return df_end(getEntryNode(pdg));
  }
};

}