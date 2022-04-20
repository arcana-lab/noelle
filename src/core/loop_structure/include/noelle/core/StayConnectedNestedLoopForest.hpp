/*
 * Copyright 2019 - 2021  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/LoopStructure.hpp"
#include "noelle/core/DominatorSummary.hpp"

namespace llvm::noelle {

  class StayConnectedNestedLoopForest ;

  class StayConnectedNestedLoopForestNode {
    public:
      StayConnectedNestedLoopForestNode (
        StayConnectedNestedLoopForest *f, 
        LoopStructure *l
        );

      StayConnectedNestedLoopForestNode (
        StayConnectedNestedLoopForest *f, 
        LoopStructure *l, 
        StayConnectedNestedLoopForestNode *parent
        );

      LoopStructure * getLoop (void) const ;

      LoopStructure * getInnermostLoopThatContains (Instruction *i) ;

      LoopStructure * getInnermostLoopThatContains (BasicBlock *bb);

      std::set<StayConnectedNestedLoopForestNode *> getNodes (void) ;

      std::set<LoopStructure *> getLoops (void) ;

      StayConnectedNestedLoopForestNode * getParent (void) const ;

      std::unordered_set<StayConnectedNestedLoopForestNode *> getChildren (void) const ;

      std::unordered_set<StayConnectedNestedLoopForestNode *> getDescendants (void) ;

      bool isIncludedInItsSubLoops (Instruction *inst) const ;

      /*
       * This function returns the total number of sub-loops contained by @this.
       * This includes the sub-loops of sub-loops.
       */
      uint32_t getNumberOfSubLoops (void) const ;

      bool visitPreOrder (
        std::function<bool (StayConnectedNestedLoopForestNode *n, uint32_t treeLevel)> funcToInvoke
        ) ;

      bool visitPostOrder (
        std::function<bool (StayConnectedNestedLoopForestNode *n, uint32_t treeLevel)> funcToInvoke
        ) ;

      ~StayConnectedNestedLoopForestNode();

    private:
      friend class StayConnectedNestedLoopForest ;
      StayConnectedNestedLoopForest *forest;
      LoopStructure *loop;
      StayConnectedNestedLoopForestNode *parent;
      std::unordered_set<StayConnectedNestedLoopForestNode *> children;

      bool visitPreOrder (std::function<bool (StayConnectedNestedLoopForestNode *n, uint32_t treeLevel)> funcToInvoke, uint32_t treeLevel) ;
      bool visitPostOrder (std::function<bool (StayConnectedNestedLoopForestNode *n, uint32_t treeLevel)> funcToInvoke, uint32_t treeLevel) ;
  };

  class StayConnectedNestedLoopForest {
    public:
      StayConnectedNestedLoopForest (
        std::vector<LoopStructure *> const & loops, 
        std::unordered_map<Function *, DominatorSummary *> const &doms
        );

      std::unordered_set<StayConnectedNestedLoopForestNode *> getTrees (void) const ;

      void removeTree (StayConnectedNestedLoopForestNode *tree) ;
      
      void addTree (StayConnectedNestedLoopForestNode *tree) ;

      StayConnectedNestedLoopForestNode * getNode (LoopStructure *loop) const ;

      StayConnectedNestedLoopForestNode * getInnermostLoopThatContains (Instruction *i) const ;

      StayConnectedNestedLoopForestNode * getInnermostLoopThatContains (BasicBlock *bb) const ;

      ~StayConnectedNestedLoopForest();

    private:
      std::unordered_map<LoopStructure *, StayConnectedNestedLoopForestNode *> nodes;
      std::unordered_set<StayConnectedNestedLoopForestNode *> trees;
      std::unordered_map<Function *, std::unordered_set<LoopStructure *>> functionLoops;
      std::unordered_map<BasicBlock *, StayConnectedNestedLoopForestNode *> headerLoops;

      void addChildrenToTree (
        StayConnectedNestedLoopForestNode *root, 
        std::unordered_map<Function *, DominatorSummary *> const &doms, 
        std::unordered_set<StayConnectedNestedLoopForestNode *> &potentialTrees
        );
  };

}
