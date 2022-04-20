/*
 * Copyright 2019 - 2022 Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/StayConnectedNestedLoopForest.hpp"

namespace llvm::noelle {

StayConnectedNestedLoopForestNode::StayConnectedNestedLoopForestNode (StayConnectedNestedLoopForest *f, LoopStructure *l)
  : StayConnectedNestedLoopForestNode(f, l, nullptr)
  {
  return ;
}

StayConnectedNestedLoopForestNode::StayConnectedNestedLoopForestNode (StayConnectedNestedLoopForest *f, LoopStructure *l, StayConnectedNestedLoopForestNode *parent)
  : forest{f}, 
    loop{l},
    parent{parent}
  {
  
  return ;
}

LoopStructure * StayConnectedNestedLoopForestNode::getLoop (void) const {
  return this->loop;
}

bool StayConnectedNestedLoopForestNode::isIncludedInItsSubLoops (Instruction *inst) const {

  /*
   * Check if the instruction is part of the loop.
   */
  if (!this->loop->isIncluded(inst)){
    return false;
  }

  /*
   * Check its children.
   */
  for (auto subLoopNode : this->children){
    auto subLoop = subLoopNode->getLoop();
    assert(subLoop != nullptr);

    /*
     * Check if the instruction belongs to the current child.
     */
    if (subLoop->isIncluded(inst)){
      return true;
    }

    /*
     * The instruction does not belong to the current child.
     */
  }

  return false;
}

uint32_t StayConnectedNestedLoopForestNode::getNumberOfSubLoops (void) const {

  /*
   * Check its children.
   */
  uint32_t subloops = 0;
  for (auto subLoop : this->children){

    /*
     * Account for the current sub-loop.
     */
    subloops++;

    /*
     * Account for the sub-loops of the current sub-loop.
     */
    subloops += subLoop->getNumberOfSubLoops();
  }

  return subloops;
}

LoopStructure * StayConnectedNestedLoopForestNode::getInnermostLoopThatContains (Instruction *i) {
  auto bb = i->getParent();
  auto ls = this->getInnermostLoopThatContains(bb);
  return ls;
}

LoopStructure * StayConnectedNestedLoopForestNode::getInnermostLoopThatContains (BasicBlock *bb) {

  /*
   * Check if the basic block is included in the current loop.
   * If it isn't, then no inner loop can contains it.
   */
  if (!this->loop->isIncluded(bb)){
    return nullptr;
  }

  /*
   * The basic block @bb is included.
   * We now need to find the innermost loop that contains it.
   */
  LoopStructure *innerLoop = nullptr;
  uint32_t innerLoopLevel = 0;
  auto f = [bb, &innerLoop, &innerLoopLevel](StayConnectedNestedLoopForestNode *n, uint32_t treeLevel) -> bool {
    auto nl = n->getLoop();
    if (!nl->isIncluded(bb)){
      return false;
    }
    if (innerLoop == nullptr){
      innerLoop = nl;
      innerLoopLevel = treeLevel;
      return false;
    }
    assert(treeLevel != innerLoopLevel);
    if (treeLevel > innerLoopLevel){
      innerLoop = nl;
      innerLoopLevel = treeLevel;
    }
    return false;
  };
  this->visitPreOrder(f);

  return innerLoop;
}

StayConnectedNestedLoopForestNode * StayConnectedNestedLoopForestNode::getParent (void) const {
  return this->parent;
}

std::unordered_set<StayConnectedNestedLoopForestNode *> StayConnectedNestedLoopForestNode::getDescendants (void) {
  std::unordered_set<StayConnectedNestedLoopForestNode *> s;

  auto f = [this, &s](StayConnectedNestedLoopForestNode *n, uint32_t treeLevel) -> bool{
    if (n == this) {
      return false;
    }
    s.insert(n);
    return false;
  };
  this->visitPreOrder(f);

  return s;
}

std::unordered_set<StayConnectedNestedLoopForestNode *> StayConnectedNestedLoopForestNode::getChildren (void) const {
  return this->children;
}

std::set<StayConnectedNestedLoopForestNode *> StayConnectedNestedLoopForestNode::getNodes (void) {
  std::set<StayConnectedNestedLoopForestNode *> s;

  auto f = [&s] (StayConnectedNestedLoopForestNode *n, uint32_t l) -> bool {
    s.insert(n);
    return false;
  };
  this->visitPreOrder(f);

  return s;
}
    
std::set<LoopStructure *> StayConnectedNestedLoopForestNode::getLoops (void) {
  std::set<LoopStructure *> s;

  auto f = [&s] (StayConnectedNestedLoopForestNode *n, uint32_t l) -> bool {
    s.insert(n->getLoop());
    return false;
  };
  this->visitPreOrder(f);

  return s;
}

bool StayConnectedNestedLoopForestNode::visitPreOrder (std::function<bool (StayConnectedNestedLoopForestNode *n, uint32_t treeLevel)> funcToInvoke) {
  return this->visitPreOrder(funcToInvoke, 1);
}

bool StayConnectedNestedLoopForestNode::visitPostOrder (std::function<bool (StayConnectedNestedLoopForestNode *n, uint32_t treeLevel)> funcToInvoke) {
  return this->visitPostOrder(funcToInvoke, 1);
}

bool StayConnectedNestedLoopForestNode::visitPreOrder (
  std::function<bool (StayConnectedNestedLoopForestNode *n, uint32_t treeLevel)> funcToInvoke, 
  uint32_t treeLevel
  ) {

  /*
   * Visit the root.
   */
  if (funcToInvoke(this, treeLevel)){
    return true ;
  }

  /*
   * Visit the children.
   */
  for (auto child : this->children){
    if (child->visitPreOrder(funcToInvoke, treeLevel + 1)){
      return true ;
    }
  }

  return false ;
}

bool StayConnectedNestedLoopForestNode::visitPostOrder (
  std::function<bool (StayConnectedNestedLoopForestNode *n, uint32_t treeLevel)> funcToInvoke, 
  uint32_t treeLevel
  ) {

  /*
   * Visit the children.
   */
  for (auto child : this->children){
    if (child->visitPostOrder(funcToInvoke, treeLevel + 1)){
      return true ;
    }
  }

  /*
   * Visit the root.
   */
  if (funcToInvoke(this, treeLevel)){
    return true ;
  }

  return false ;
}

StayConnectedNestedLoopForestNode::~StayConnectedNestedLoopForestNode(){

  /*
   * Check if this object is an internal node of a tree.
   */
  if (this->parent != nullptr){

    /*
     * The node is internal.
     *
     * Remove the current node from the descendant of the parent.
     */
    assert(this->parent->children.find(this) != this->parent->children.end());
    this->parent->children.erase(this);

    /*
     * Add the children of @this as immediate children to the parent.
     */
    for (auto child : this->children){
      child->parent = this->parent;
      this->parent->children.insert(child);
    }

    return ;
  }

  /*
   * The object is the root of a tree.
   *
   * Unregister @this as tree.
   */
  this->forest->removeTree(this);

  /*
   * Promote all children to trees of the forest.
   */
  for (auto child : this->children){
    child->parent = nullptr;
    this->forest->addTree(child);
  }

  return ;
}

}
