/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni
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

#include "noelle/core/DominatorSummary.hpp"

namespace llvm::noelle {

/*
 * Dominator Node Summary implementation
 */

DomNodeSummary::DomNodeSummary(const DTAliases::Node &node)
  : B{ node.getBlock() },
    level{ node.getLevel() },
    parent{ nullptr },
    iDom{ nullptr },
    children{} {}

DomNodeSummary::DomNodeSummary(const DomNodeSummary &node)
  : B{ node.getBlock() },
    level{ node.getLevel() },
    parent{ nullptr },
    iDom{ nullptr },
    children{} {}

raw_ostream &DomNodeSummary::print(raw_ostream &stream, std::string prefix) {
  stream << prefix << "Block: ";
  if (getBlock())
    getBlock()->printAsOperand(stream);
  else
    stream << "null";
  stream << " Level: " << getLevel() << " Parent: ";
  if (getParent() && getParent()->getBlock())
    getParent()->getBlock()->printAsOperand(stream);
  else
    stream << "null";
  stream << " I Dom: ";
  if (getIDom() && getIDom()->getBlock())
    getIDom()->getBlock()->printAsOperand(stream);
  else
    stream << "null";
  stream << "\n" << prefix << "Children: ";
  for (auto child : getChildren()) {
    child->getBlock()->printAsOperand(stream << "\t");
  }
  return stream << "\n";
}

BasicBlock *DomNodeSummary::getBlock(void) const {
  return B;
}

DomNodeSummary *DomNodeSummary::getParent(void) {
  return parent;
}

std::vector<DomNodeSummary *> DomNodeSummary::getChildren(void) {
  return children;
}

unsigned DomNodeSummary::getLevel(void) const {
  return level;
}

DomNodeSummary *DomNodeSummary::getIDom(void) {
  return iDom;
}

/*
 * Dominator Tree Summary implementation
 */

DomTreeSummary::DomTreeSummary(DominatorTree &DT)
  : DomTreeSummary{ collectNodesOfTree<DominatorTree>(DT) } {
  this->post = false;
  return;
}

DomTreeSummary::DomTreeSummary(PostDominatorTree &PDT)
  : DomTreeSummary{ collectNodesOfTree<PostDominatorTree>(PDT) } {
  this->post = true;
  return;
}

DomTreeSummary::DomTreeSummary(std::set<DTAliases::Node *> nodeSubset)
  : nodes{},
    bbNodeMap{} {
  this->cloneNodes<DTAliases::Node>(nodeSubset);
}

DomTreeSummary::DomTreeSummary(DomTreeSummary &DTS,
                               std::set<BasicBlock *> &bbSubset)
  : DomTreeSummary{ filterNodes(DTS.nodes, bbSubset) } {}

DomTreeSummary::DomTreeSummary(std::set<DomNodeSummary *> nodeSubset)
  : nodes{},
    bbNodeMap{} {
  this->cloneNodes<DomNodeSummary>(nodeSubset);
}

DomTreeSummary::~DomTreeSummary() {
  for (auto node : nodes)
    delete node;
  nodes.clear();
  bbNodeMap.clear();
}

void DomTreeSummary::transferToClones(
    std::unordered_map<BasicBlock *, BasicBlock *> &bbCloneMap) {
  for (auto node : nodes) {
    assert(bbCloneMap.find(node->B) != bbCloneMap.end());
    node->B = bbCloneMap[node->B];
  }
}

template <typename TreeType>
std::set<DTAliases::Node *> DomTreeSummary::collectNodesOfTree(TreeType &T) {
  std::set<DTAliases::Node *> nodes;
  std::vector<DTAliases::Node *> worklist;
  auto &rootBlocks = T.getRoots();
  for (BasicBlock *b : rootBlocks)
    worklist.push_back(T.getNode(b));

  /*
   * Workaround: An empty "exit node" exists for PostDominatorTree that isn't
   * accessible via getRoots()
   */
  worklist.push_back(T.getRootNode());

  while (worklist.size() != 0) {
    auto node = worklist.back();
    worklist.pop_back();
    nodes.insert(node);
    auto children = node->getChildren();
    for (auto child : children)
      worklist.push_back(child);
  }

  return nodes;
}

std::set<DomNodeSummary *> DomTreeSummary::filterNodes(
    std::set<DomNodeSummary *> &nodes,
    std::set<BasicBlock *> &bbSubset) {
  std::set<DomNodeSummary *> nodesSubset;
  for (auto node : nodes) {
    if (bbSubset.find(node->B) != bbSubset.end()) {
      nodesSubset.insert(node);
    }
  }
  return nodesSubset;
}

template <typename NodeType>
void DomTreeSummary::cloneNodes(std::set<NodeType *> &nodesToClone) {

  /*
   * Clone nodes using DomNodeSummary constructors. Track cloned pairs in map
   */
  std::unordered_map<NodeType *, DomNodeSummary *> nodeMap;
  for (auto node : nodesToClone) {
    auto summary = new DomNodeSummary(*node);
    nodeMap[node] = summary;
    this->nodes.insert(summary);
    this->bbNodeMap[summary->B] = summary;
  }

  /*
   * Populate parent, child relations between cloned nodes.
   * Note the optional nature of these connections. It is possible
   * that only a subset of the tree is being cloned
   */
  for (auto node : nodesToClone) {
    auto summary = nodeMap[node];
    auto iDom = node->getIDom();
    if (nodeMap.find(iDom) != nodeMap.end()) {
      summary->iDom = nodeMap[iDom];
    }

    auto children = node->getChildren();
    for (auto child : children) {
      if (nodeMap.find(child) == nodeMap.end())
        continue;
      auto childSummary = nodeMap[child];
      childSummary->parent = summary;
      summary->children.push_back(childSummary);
    }
  }
}

DomNodeSummary *DomTreeSummary::getNode(BasicBlock *B) const {
  auto nodeIter = bbNodeMap.find(B);
  return nodeIter == bbNodeMap.end() ? nullptr : nodeIter->second;
}

bool DomTreeSummary::dominates(Instruction *I, Instruction *J) const {
  auto B1 = I->getParent();
  auto B2 = J->getParent();

  /*
   * Check if the instructions belong to the same basic block.
   */
  if (B1 == B2) {

    /*
     * Define the first and second instruction.
     */
    auto firstOne = I;
    auto secondOne = J;

    /*
     * Check the dominance relation between I and J
     */
    while (firstOne != nullptr) {
      if (firstOne == secondOne) {

        /*
         * The secondOne instruction is found after the first one.
         * Hence, I dominates J.
         * Also, J postdominates I.
         */
        if (this->post) {

          /*
           * I does not post-dominate J.
           */
          return false;
        }

        /*
         * I dominates J
         */
        return true;
      }

      firstOne = firstOne->getNextNode();
    }

    /*
     * The secondOne instruction has not been found after the first one.
     * Hence, J dominates I.
     * Also, I post-dominates J.
     */
    if (this->post) {

      /*
       * I post-dominates J.
       */
      return true;
    }

    /*
     * I does not dominates J.
     */
    return false;
  }

  /*
   * The instructions belong to different basic blocks.
   *
   * Check if B1 dominates B2.
   */
  auto d = this->dominates(B1, B2);

  return d;
}

bool DomTreeSummary::dominates(BasicBlock *B1, BasicBlock *B2) const {
  auto nodeB1 = this->getNode(B1);
  auto nodeB2 = this->getNode(B2);
  assert(
      nodeB1 && nodeB2
      && "The basic blocks provided to DomTreeSummary are not present in the tree");
  return this->dominates(nodeB1, nodeB2);
}

bool DomTreeSummary::dominates(DomNodeSummary *node1,
                               DomNodeSummary *node2) const {
  std::queue<DomNodeSummary *> worklist;
  worklist.push(node1);
  while (!worklist.empty()) {
    auto node = worklist.front();
    worklist.pop();

    if (node == node2)
      return true;
    for (auto child : node->children)
      worklist.push(child);
  }
  return false;
}

std::set<DomNodeSummary *> DomTreeSummary::dominates(
    DomNodeSummary *node) const {
  std::set<DomNodeSummary *> dominators;
  while (node->parent) {
    dominators.insert(node);
    node = node->parent;
  }
  return dominators;
}

BasicBlock *DomTreeSummary::findNearestCommonDominator(BasicBlock *B1,
                                                       BasicBlock *B2) const {
  assert(B1 != nullptr);
  assert(B2 != nullptr);

  /*
   * Fetch the nodes in the dominator tree.
   */
  auto n1 = this->getNode(B1);
  auto n2 = this->getNode(B2);
  assert(n1 != nullptr);
  assert(n2 != nullptr);

  /*
   * Find the nearest common dominator.
   */
  auto c = findNearestCommonDominator(n1, n2);
  assert(c != nullptr);

  return c->B;
}

DomNodeSummary *DomTreeSummary::findNearestCommonDominator(
    DomNodeSummary *node1,
    DomNodeSummary *node2) const {

  /*
   * Helpers to determine whether a node n dominates node2
   */
  auto dominatorsOf2 = this->dominates(node2);
  auto dominates2 = [&](DomNodeSummary *node) -> bool {
    return dominatorsOf2.find(node) != dominatorsOf2.end();
  };

  /*
   * Traversal of parents of node1 to find common dominator
   */
  DomNodeSummary *node = node1;
  while (node && !dominates2(node))
    node = node->parent;
  return node;
}

raw_ostream &DomTreeSummary::print(raw_ostream &stream,
                                   std::string prefixToUse) const {
  for (auto node : nodes) {
    node->print(stream, prefixToUse);
  }
  return stream;
}

DominatorSummary::DominatorSummary(DominatorTree &dt, PostDominatorTree &pdt)
  : DT{ dt },
    PDT{ pdt } {
  return;
}

DominatorSummary::DominatorSummary(DominatorSummary &ds,
                                   std::set<BasicBlock *> &bbSubset)
  : DT{ ds.DT, bbSubset },
    PDT{ ds.PDT, bbSubset } {}

void DominatorSummary::transferSummaryToClones(
    std::unordered_map<BasicBlock *, BasicBlock *> &bbCloneMap) {
  DT.transferToClones(bbCloneMap);
  PDT.transferToClones(bbCloneMap);
}

} // namespace llvm::noelle
