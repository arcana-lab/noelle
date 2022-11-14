/*
 *
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "noelle/core/DominatorNode.hpp"

namespace llvm::noelle {

class DominatorForest {
public:
  DominatorForest(llvm::DominatorTree &DT);

  DominatorForest(llvm::PostDominatorTree &PDT);

  DominatorForest(DominatorForest &DTS, std::set<BasicBlock *> &bbSubset);

  void transferToClones(
      std::unordered_map<BasicBlock *, BasicBlock *> &bbCloneMap);

  DominatorNode *getNode(BasicBlock *B) const;
  bool dominates(Instruction *I, Instruction *J) const;
  bool dominates(BasicBlock *B1, BasicBlock *B2) const;
  bool dominates(DominatorNode *node1, DominatorNode *node2) const;
  std::set<DominatorNode *> dominates(DominatorNode *node) const;
  BasicBlock *findNearestCommonDominator(BasicBlock *B1, BasicBlock *B2) const;
  DominatorNode *findNearestCommonDominator(DominatorNode *node1,
                                            DominatorNode *node2) const;

  std::set<Instruction *> getDominatorsOf(const std::set<Instruction *> &s,
                                          BasicBlock *dominatedBB) const;
  std::set<Instruction *> getInstructionsThatDoNotDominateAnyOther(
      const std::set<Instruction *> &s) const;

  raw_ostream &print(raw_ostream &stream, std::string prefixToUse = "") const;

  ~DominatorForest();

private:
  std::set<DominatorNode *> nodes;
  std::unordered_map<BasicBlock *, DominatorNode *> bbNodeMap;
  bool post;

  DominatorForest(std::set<DTAliases::Node *> nodes);
  DominatorForest(std::set<DominatorNode *> nodesSubset);
  template <typename TreeType>
  std::set<DTAliases::Node *> collectNodesOfTree(TreeType &T);
  std::set<DominatorNode *> filterNodes(std::set<DominatorNode *> &nodes,
                                        std::set<BasicBlock *> &bbSubset);
  template <typename NodeType>
  void cloneNodes(std::set<NodeType *> &nodes);
};

} // namespace llvm::noelle
