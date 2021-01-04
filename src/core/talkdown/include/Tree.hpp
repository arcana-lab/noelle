#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Analysis/LoopInfo.h"

#include "scaf/Utilities/ModuleLoops.h"
#include "Node.hpp"
#include "Annotation.hpp"

#include <string>

namespace AutoMP {


  class FunctionTree
  {
  public:
    // constructors
    FunctionTree(llvm::Function *f) : root(nullptr), num_nodes(0), associated_function(f) { }
    FunctionTree() : FunctionTree(nullptr) { }
    ~FunctionTree();

    bool constructTree(llvm::Function *f, llvm::LoopInfo &li);

    llvm::Function *getFunction(void) const { return associated_function; }
    std::set<Node *> getLeaves() const;
    Node* getRoot() const;

    // printing stuff
    void printNodeToInstructionMap(void) const;
    friend std::ostream &operator<<(std::ostream &, const FunctionTree &);
    friend llvm::raw_ostream &operator<<(llvm::raw_ostream &, const FunctionTree &);

    const AnnotationSet &getAnnotationsForInst(llvm::Instruction *) const;
    const AnnotationSet &getAnnotationsForInst(llvm::Instruction *, llvm::Loop *) const;

    bool loopContainsAnnotation(llvm::Loop *) const;

    void writeDotFile(const std::string filename);

    bool isValidTree(void) const;

    std::vector<Node *> nodes; // remove this once an iterator is developed

  private:
    llvm::Function *associated_function;
    Node *root;

    int num_nodes;

    Node *findInst(Node*, llvm::Instruction *) const;
    Node *findNodeForLoop(Node *, llvm::Loop *) const; // level-order traversal
    Node *findNodeForBasicBlock(Node *, llvm::BasicBlock *) const;
    Node *findNodeForInstruction(Node *, llvm::Instruction *) const;
    Node *searchUpForAnnotation(Node *start, std::pair<std::string, std::string> a) __attribute__ ((deprecated)); // search upward from a node to find first node with matching annotation
    std::vector<Node *> getNodesInPreorder(Node *start) const;
    std::vector<LoopContainerNode *> getAllLoopContainerNodes(void) const;
    std::vector<Node *> getAllLoopBasicBlockNodes(void) const;

    void addLoopContainersToTree(llvm::LoopInfo &li);
    void annotateBasicBlocks(void);
    void annotateLoops(void);

    // Change these to use the node instead so we don't have to traverse the tree?
    void addBasicBlocksToLoops(llvm::LoopInfo &li);
    void addNonLoopBasicBlocks(llvm::LoopInfo &li);

    bool splitBasicBlocksByAnnotation(void);
    bool fixBasicBlockAnnotations(void);

    bool handleCriticalAnnotations(void);
    bool handleOwnedAnnotations(void);

  public:
    // XXX: First attempt at creating custom iterator. Maybe at a const_iterator later?
    // I have no idea what I'm doing lol
    class iterator
    {
      Node *start;
      iterator(Node *n) : start(n) {}
      /* iterator &operator++() {} */
    };


  };

  // Things to consider:
  //  - how to implement an efficient iterator? If we want to traverse the tree often, then we should
  //    use a std::set and add to it every time a node is added (probably worth it). Otherwise, manually
  //    do a level-order traversal every time????

} // namespace AutoMP
