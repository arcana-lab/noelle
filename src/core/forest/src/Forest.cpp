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

StayConnectedNestedLoopForest::StayConnectedNestedLoopForest (
  std::vector<LoopStructure *> const & loops,
  std::unordered_map<Function *, DominatorSummary *> const &doms
  ){

  /*
   * Allocate the nodes.
   */
  std::unordered_set<StayConnectedNestedLoopForestNode *> potentialTrees{};
  for (auto l : loops){
    auto func = l->getFunction();
    auto header = l->getHeader();
    auto n = new StayConnectedNestedLoopForestNode(this, l);
    this->nodes[l] = n;
    this->functionLoops[func].insert(l);
    this->headerLoops[header] = n;

    /*
     * Check if the current loop is an outermost one.
     * If it is, then this loop is a tree.
     */
    if (l->getNestingLevel() == 1){
      this->trees.insert(n);

    } else {
      potentialTrees.insert(n);
    }
  }

  /*
   * Fill up the trees.
   */
  for (auto t : this->trees){

    /*
     * Add the children recursively.
     */
    this->addChildrenToTree(t, doms, potentialTrees);
  }

  /*
   * The loops that haven't been attached to a tree become trees.
   */
  for (auto n : potentialTrees){
    this->trees.insert(n);
  }
  
  return ;
}
      
void StayConnectedNestedLoopForest::addChildrenToTree (
  StayConnectedNestedLoopForestNode *root,
  std::unordered_map<Function *, DominatorSummary *> const &doms,
  std::unordered_set<StayConnectedNestedLoopForestNode *> &potentialTrees
  ){

  /*
   * Fetch the loop.
   */
  auto l = root->getLoop();
  auto header = l->getHeader();
  auto func = l->getFunction();

  /*
   * Fetch the dominators.
   */
  auto ds = doms.at(func);
  auto loopHeaderDominatorNode = ds->DT.getNode(header);

  /*
   * Add the children.
   */
  for (auto functionLoop : this->functionLoops[func]){

    /*
     * Fetch the header of the current loop that belongs to the same function of @root.
     */
    auto functionLoopHeader = functionLoop->getHeader();

    /*
     * Do not consider @root.
     */
    if (functionLoopHeader == header){
      continue ;
    }

    /*
     * Check if the current loop of the same function of @root is included in @root.
     */
    if (!l->isIncluded(functionLoopHeader)){
      continue ;
    }

    /*
     * The loop @functionLoop is included in @root.
     *
     * Check if @functionLoop is the immediate descendent of @root among the loops given as input.
     * Notice that this doesn't necessary mean @functionLoop must be an immediate sub-loop of @root.
     *
     * To this end, we traverse the dominator tree from @functionLoopHeader to @header. 
     * If we find another loop in between, then @functionLoop isn't the immediate descendant of @root.
     * Otherwise, it is.
     */
    auto subLoopDominatorNode = ds->DT.getNode(functionLoopHeader);
    assert(loopHeaderDominatorNode != subLoopDominatorNode);

    /*
     * Start the search from the immediate dominator of the header of @functionLoopHeader.
     */
    subLoopDominatorNode = subLoopDominatorNode->getParent();
    auto foundLoopInBetween = false;
    while (loopHeaderDominatorNode != subLoopDominatorNode){

      /*
       * Check if the current basic block is the header of a loop.
       */
      auto bb = subLoopDominatorNode->getBlock();
      if (this->headerLoops.find(bb) != this->headerLoops.end()){

        /*
         * We have found a loop in between @root and @functionLoopHeader among the set given as input.
         *
         * Check if the loop in between includes @functionLoopHeader.
         */
        auto loopInBetween = this->headerLoops.at(bb);
        auto lsLoopInBetween = loopInBetween->getLoop();
        if (lsLoopInBetween->isIncluded(functionLoopHeader)){

          /*
           * The loop in between includes @functionLoopHeader.
           * Hence, @functionLoopHeader isn't an immediate descendant of @root.
           */
          foundLoopInBetween = true;
          break ;
        }
      }

      /*
       * Keep traversing the dominator tree.
       */
      subLoopDominatorNode = subLoopDominatorNode->getParent();
    }
    if (foundLoopInBetween){

      /*
       * We have found a loop in between @root and @functionLoopHeader among the set given as input.
       * Hence, @functionLoopHeader isn't an immediate descendant of @root.
       */
      continue ;
    }

    /*
     * Add the child.
     */
    auto c = this->nodes[functionLoop];
    assert(c != nullptr);
    root->children.insert(c);
    c->parent = root;
    potentialTrees.erase(c);

    /*
     * Go deeper recursively.
     */
    this->addChildrenToTree(c, doms, potentialTrees);
  }

  return ;
}
    
std::unordered_set<StayConnectedNestedLoopForestNode *> StayConnectedNestedLoopForest::getTrees (void) const {
  return this->trees;
}

void StayConnectedNestedLoopForest::removeTree (StayConnectedNestedLoopForestNode *tree) {
  assert(this->trees.find(tree) != this->trees.end());
  this->trees.erase(tree);
  assert(this->trees.find(tree) == this->trees.end());

  return ;
}
    
void StayConnectedNestedLoopForest::addTree (StayConnectedNestedLoopForestNode *tree) {
  assert(this->trees.find(tree) == this->trees.end());

  this->trees.insert(tree);

  return ;
}

StayConnectedNestedLoopForest::~StayConnectedNestedLoopForest (){
  for (auto pair : this->nodes){
    delete pair.second ;
  }
}
  
StayConnectedNestedLoopForestNode * StayConnectedNestedLoopForest::getNode (LoopStructure *loop) const {

  /*
   * Fetch the header
   */
  auto h = loop->getHeader();

  /*
   * Fetch the node.
   */
  if (this->headerLoops.find(h) == this->headerLoops.end()){
    return nullptr;
  }
  auto n = this->headerLoops.at(h);
  return n;
}

StayConnectedNestedLoopForestNode * StayConnectedNestedLoopForest::getInnermostLoopThatContains (Instruction *i) const {
  StayConnectedNestedLoopForestNode *n = nullptr;

  /*
   * Identify only the trees that are about the same function of the target instruction.
   */
  for (auto tree : this->getTrees()){
    
    /*
     * Fetch the function of the tree
     */
    auto ls = tree->getLoop();
    auto treeFunction = ls->getFunction();

    /*
     * Skip trees about functions that do not include the target instruction.
     */
    if (treeFunction != i->getFunction()){
      continue ;
    }

    /*
     * The current tree is of the same function of the target instruction.
     *
     * Check if the current tree includes the target instruction.
     */
    if (!ls->isIncluded(i)){

      /*
       * The current tree doesn't include the target instruction.
       */
      continue ;
    }

    /*
     * The current tree includes the target instruction.
     *
     * Fetch the innermost loop that contains it.
     */
    StayConnectedNestedLoopForestNode *innermostLoop = nullptr;
    auto finderFunction = [i, &innermostLoop](StayConnectedNestedLoopForestNode *n, uint32_t treeLevel) -> bool {

      /*
       * Fetch the loop
       */
      auto loop = n->getLoop();
      if (loop->isIncluded(i)){
        innermostLoop = n;
        return true;
      }
      return false;
    };
    tree->visitPostOrder(finderFunction);
    assert(innermostLoop != nullptr);
    return innermostLoop;
  }

  return n;
}

StayConnectedNestedLoopForestNode * StayConnectedNestedLoopForest::getInnermostLoopThatContains (BasicBlock *bb) const {
  auto n = this->getInnermostLoopThatContains(&*bb->begin());
  return n;
}

}
