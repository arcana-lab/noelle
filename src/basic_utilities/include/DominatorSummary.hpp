/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"

namespace llvm {

  namespace DTAliases {
    using Node = DomTreeNodeBase<BasicBlock>;
  }

  class DomNodeSummary {
   public:
    DomNodeSummary (const DTAliases::Node &node);
    DomNodeSummary (const DomNodeSummary &node);

    BasicBlock *getBlock (void) const ;
    DomNodeSummary *getParent (void) ;
    std::vector<DomNodeSummary *> getChildren (void) ;
    unsigned getLevel (void) const ;
    DomNodeSummary *getIDom (void) ;

    raw_ostream &print (raw_ostream &stream, std::string prefixToUse = "") ;

    friend class DomTreeSummary;

   private:
    BasicBlock *B;
    unsigned level;

    DomNodeSummary *parent;
    std::vector<DomNodeSummary *> children;
    DomNodeSummary *iDom;
  };

  class DomTreeSummary {
   public:
    DomTreeSummary (DominatorTree &DT);
    DomTreeSummary (PostDominatorTree &PDT);
    DomTreeSummary (DomTreeSummary &DTS, std::set<BasicBlock *> &bbSubset);
    ~DomTreeSummary ();

    void transferToClones (std::unordered_map<BasicBlock *, BasicBlock *> &bbCloneMap);
    raw_ostream &print (raw_ostream &stream, std::string prefixToUse = "") const ;

   private:
    DomTreeSummary (std::set<DTAliases::Node *> nodes);
    DomTreeSummary (std::set<DomNodeSummary *> nodesSubset);

    template <typename TreeType>
    std::set<DTAliases::Node *> collectNodesOfTree (TreeType &T);
    std::set<DomNodeSummary *> filterNodes (
      std::set<DomNodeSummary *> &nodes,
      std::set<BasicBlock *> &bbSubset
    );
    template <typename NodeType>
    void cloneNodes (std::set<NodeType *> &nodes);

   public:
    DomNodeSummary *getNode (BasicBlock *B) const ;
    bool dominates (Instruction *I, Instruction *J) const ;
    bool dominates (BasicBlock *B1, BasicBlock *B2) const ;
    bool dominates (DomNodeSummary *node1, DomNodeSummary *node2) const ;
    std::set<DomNodeSummary *> dominates (DomNodeSummary *node) const ;
    BasicBlock *findNearestCommonDominator (BasicBlock *B1, BasicBlock *B2) const ;
    DomNodeSummary *findNearestCommonDominator (DomNodeSummary *node1, DomNodeSummary *node2) const ;

   private:
    std::set<DomNodeSummary *> nodes;
    std::unordered_map<BasicBlock *, DomNodeSummary *> bbNodeMap;
  };

  class DominatorSummary {
   public:
    DominatorSummary (DominatorTree &DT, PostDominatorTree &PDT);
    DominatorSummary (DominatorSummary &DS, std::set<BasicBlock *> &bbSubset);

    void transferSummaryToClones (std::unordered_map<BasicBlock *, BasicBlock *> &bbCloneMap);

    DomTreeSummary DT, PDT;
  };

}
