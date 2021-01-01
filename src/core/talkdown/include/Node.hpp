#pragma once

//#include "LoopStructure.hpp"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/Casting.h"
#include "llvm/Analysis/LoopInfo.h"

#include <array>
#include <set>
#include <unordered_set>

#include "Annotation.hpp"

namespace AutoMP {

  struct Node
  {
  // Set up an enum to work with llvm's dyn_cast()
  public:
    enum NodeKind
    {
      NK_Base,
      NK_LoopContainer,
    };
    NodeKind getKind() const { return kind; }
    static bool classof(const Node *n) {
      return n->getKind() >= NK_Base && n->getKind() <= NK_LoopContainer;
    }

  // constructors and destructors
  public:
    Node() : Node(nullptr, nullptr, nullptr) { }
		Node(Node *p, llvm::Loop *l = nullptr, llvm::BasicBlock *bb = nullptr) : Node(p, l, bb, NK_Base) {}
		Node(Node *p, llvm::Loop *l, llvm::BasicBlock *bb, NodeKind k) :
			parent(p), loop(l), basic_block(bb), kind(k), location_string()
    {
      if ( p )
        p->addChild(this);
      setID(how_many);
      how_many++;
    }
		virtual ~Node() { how_many--; }

  // class methods
  public:
    // Linking nodes together
    Node *getParent(void) const { return parent; }
    void setParent(Node *p) { parent = p; }
    void addChild(Node *p) { children.emplace(p); }
    void removeChild(Node *p) { children.erase(p); }
    const std::set<Node *> &getChildren(void) const { return children; }

    // Getting and setting data of nodes
    void setID(int i) { ID = i; }
    int getID(void) const { return ID; }
    void setLoop(llvm::Loop *l) { loop = l; }
    llvm::Loop *getLoop(void) const { return loop; }
    void setBB(llvm::BasicBlock *bb) { basic_block = bb; }
    llvm::BasicBlock *getBB(void) const { return basic_block; }

    // Dealing with annotations
    void replaceAnnotations(AnnotationSet &&as) { annotations = as; }
    void replaceAnnotationsWithKey(AnnotationSet &&);
    void addAnnotations(AnnotationSet &&);
    bool containsAnnotationWithKey(std::string s) const;
    bool containsAnnotation(const Annotation &a) const;
    const AnnotationSet &getAnnotations(void) const { return annotations; }
    AnnotationSet getRealAnnotations(void) const;

    // debug locations in source
    void setDebugLoc(std::string s) { location_string = s; }
    std::string getDebugLoc() const { return location_string; }

    // printing stuff
    llvm::raw_ostream &recursivePrint(llvm::raw_ostream &) const;
    friend std::ostream &operator<<(std::ostream &, const Node *);
    friend llvm::raw_ostream &operator<<(llvm::raw_ostream &, const Node *);

    // TODO: make this private
    AnnotationSet annotations;

		static unsigned int how_many;

  private:
    unsigned int ID;
    Node *parent;
    std::set<Node *> children;
    llvm::Loop *loop;
    llvm::BasicBlock *basic_block;
    const NodeKind kind;
    std::string location_string; // location in source code

    static constexpr std::array restricted_keys{
      "__root",
      "__loop_container",
      "__level",
      "__non_loop_bb",
      "__loop_bb",
      "__loop_header",
    };
  };

  // XXX To be used later
  struct LoopContainerNode : public Node
  {
  public:
    LoopContainerNode() : LoopContainerNode(nullptr, nullptr) { }
    LoopContainerNode(Node *p, llvm::Loop *l) :
      Node(p, l, nullptr, NK_LoopContainer) { }

    void setHeaderNode(const Node *n) { header = const_cast<Node *>(n); }
    Node *getHeaderNode(void) const { return header; }

    /* std::set<Node *> getBasicBlockNodes(void) { */
    /*   for ( auto &n : children ) */
    /*   { */
    /*     if ( isa<LoopContainerNode>(n) ) */
    /*       continue; */
    /*   } */
    /* } */

    static bool classof(const Node *n) {
      return n->getKind() == NK_LoopContainer;
    }

  private:
    // basic blocks contained within this loop (including subloops)
    // used to speed up finding basic blocks?
    std::unordered_set<llvm::BasicBlock *> contained_bbs;

    // node representing header basic block
    Node *header;
  };

  // XXX Not used currently
  // All children of a BasicBlockContainerNode have the same annotation and belong
  // to the same loop.
  // It does however add some unnecessary nodes so maybe not use this...
  struct BasicBlockContainerNode : public Node
  {
  public:
    BasicBlockContainerNode()
    {
      assert(0 && "BasicBlockContainerNode not implemented yet");
    }
  private:
  };
} // namespace llvm
