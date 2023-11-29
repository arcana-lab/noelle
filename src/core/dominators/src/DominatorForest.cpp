/*
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
#include "noelle/core/DominatorForest.hpp"

namespace arcana::noelle {

DominatorForest::DominatorForest(llvm::DominatorTree &DT)
  : DominatorForest{ collectNodesOfTree<llvm::DominatorTree>(DT) } {
  this->post = false;
  return;
}

DominatorForest::DominatorForest(llvm::PostDominatorTree &PDT)
  : DominatorForest{ collectNodesOfTree<PostDominatorTree>(PDT) } {
  this->post = true;
  return;
}

DominatorForest::DominatorForest(std::set<DTAliases::Node *> nodeSubset)
  : nodes{},
    bbNodeMap{} {
  this->cloneLLVMNodes(nodeSubset);
  return;
}

DominatorForest::DominatorForest(DominatorForest &DTS,
                                 std::set<BasicBlock *> &bbSubset)
  : DominatorForest{ filterNodes(DTS.nodes, bbSubset) } {
  return;
}

DominatorForest::DominatorForest(std::set<DominatorNode *> nodeSubset)
  : nodes{},
    bbNodeMap{} {
  this->cloneNodes<DominatorNode>(nodeSubset);
  return;
}

DominatorForest::~DominatorForest() {
  for (auto node : nodes)
    delete node;
  nodes.clear();
  bbNodeMap.clear();
}

void DominatorForest::transferToClones(
    std::unordered_map<BasicBlock *, BasicBlock *> &bbCloneMap) {
  for (auto node : nodes) {
    assert(bbCloneMap.find(node->B) != bbCloneMap.end());
    node->B = bbCloneMap[node->B];
  }
}

template <typename TreeType>
std::set<DTAliases::Node *> DominatorForest::collectNodesOfTree(TreeType &T) {
  std::set<DTAliases::Node *> nodes;
  std::vector<DTAliases::Node *> worklist;
  for (BasicBlock *b : T.roots())
    worklist.push_back(T.getNode(b));

  /*
   * Workaround: An empty "exit node" exists for PostDominatorForest that isn't
   * accessible via getRoots()
   */
  worklist.push_back(T.getRootNode());

  while (worklist.size() != 0) {
    auto node = worklist.back();
    worklist.pop_back();
    nodes.insert(node);
    for (auto child : *node)
      worklist.push_back(child);
  }

  return nodes;
}

std::set<DominatorNode *> DominatorForest::filterNodes(
    std::set<DominatorNode *> &nodes,
    std::set<BasicBlock *> &bbSubset) {
  std::set<DominatorNode *> nodesSubset;
  for (auto node : nodes) {
    if (bbSubset.find(node->B) != bbSubset.end()) {
      nodesSubset.insert(node);
    }
  }
  return nodesSubset;
}

void DominatorForest::cloneLLVMNodes(
    std::set<DTAliases::Node *> &nodesToClone) {

  /*
   * Clone nodes using DomNodeSummary constructors. Track cloned pairs in map
   */
  std::unordered_map<DTAliases::Node *, DominatorNode *> nodeMap;
  for (auto node : nodesToClone) {
    auto summary = new DominatorNode(*node);
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

    for (auto child : node->children()) {
      if (nodeMap.find(child) == nodeMap.end())
        continue;
      auto childSummary = nodeMap[child];
      childSummary->parent = summary;
      summary->children.push_back(childSummary);
    }
  }

  return;
}

template <typename NodeType>
void DominatorForest::cloneNodes(std::set<NodeType *> &nodesToClone) {

  /*
   * Clone nodes using DominatorNode constructors. Track cloned pairs in map
   */
  std::unordered_map<NodeType *, DominatorNode *> nodeMap;
  for (auto node : nodesToClone) {
    auto summary = new DominatorNode(*node);
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

    for (auto child : node->getChildren()) {
      if (nodeMap.find(child) == nodeMap.end())
        continue;
      auto childSummary = nodeMap[child];
      childSummary->parent = summary;
      summary->children.push_back(childSummary);
    }
  }
}

DominatorNode *DominatorForest::getNode(BasicBlock *B) const {
  auto nodeIter = bbNodeMap.find(B);
  return nodeIter == bbNodeMap.end() ? nullptr : nodeIter->second;
}

bool DominatorForest::dominates(Instruction *I, Instruction *J) const {
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

bool DominatorForest::dominates(BasicBlock *B1, BasicBlock *B2) const {
  auto nodeB1 = this->getNode(B1);
  auto nodeB2 = this->getNode(B2);
  assert(
      nodeB1 && nodeB2
      && "The basic blocks provided to DominatorForest are not present in the tree");
  return this->dominates(nodeB1, nodeB2);
}

bool DominatorForest::strictlyDominates(Instruction *I, Instruction *J) const {
  if (I == J) {
    return false;
  }

  return this->dominates(I, J);
}

bool DominatorForest::strictlyDominates(BasicBlock *B1, BasicBlock *B2) const {
  if (B1 == B2) {
    return false;
  }

  return this->dominates(B1, B2);
}

bool DominatorForest::dominates(DominatorNode *node1,
                                DominatorNode *node2) const {
  std::queue<DominatorNode *> worklist;
  worklist.push(node1);
  while (!worklist.empty()) {
    auto node = worklist.front();
    worklist.pop();

    if (node == node2) {
      return true;
    }
    for (auto child : node->children)
      worklist.push(child);
  }

  return false;
}

std::set<DominatorNode *> DominatorForest::dominates(
    DominatorNode *node) const {
  std::set<DominatorNode *> dominators;
  while (node) {
    dominators.insert(node);
    node = node->parent;
  }
  return dominators;
}

std::set<Instruction *> DominatorForest::getDominatorsOf(
    const std::set<Instruction *> &s,
    BasicBlock *dominatedBB) const {
  std::set<Instruction *> r{};

  /*
   * Consider all elements of the set.
   */
  for (auto value : s) {

    /*
     * Check if @value dominates @dominatedBB
     */
    auto valueBB = value->getParent();
    if (this->dominates(valueBB, dominatedBB)) {
      r.insert(value);
    }
  }

  return r;
}

std::set<BasicBlock *> DominatorForest::getDescendants(BasicBlock *bb) const {
  std::set<BasicBlock *> ds;

  /*
   * Fetch the node that represents @bb.
   */
  auto bbNode = this->getNode(bb);
  assert(bbNode != nullptr);

  this->addDescendants(bbNode, ds);

  return ds;
}

void DominatorForest::addDescendants(DominatorNode *n,
                                     std::set<BasicBlock *> &ds) const {

  /*
   * Add itself.
   */
  ds.insert(n->getBlock());

  /*
   * Iterate over children.
   */
  for (auto child : n->getChildren()) {
    this->addDescendants(child, ds);
  }

  return;
}

std::set<Instruction *> DominatorForest::
    getInstructionsThatDoNotDominateAnyOther(
        const std::set<Instruction *> &s) const {
  std::set<Instruction *> r{};

  /*
   * Consider all elements of the set.
   */
  for (auto value : s) {

    /*
     * Check if @value dominates any other
     */
    auto isDominatingOthers = false;
    for (auto otherValue : s) {
      if (value == otherValue) {
        continue;
      }
      if (!this->dominates(value, otherValue)) {
        continue;
      }
      isDominatingOthers = true;
      break;
    }
    if (isDominatingOthers) {
      continue;
    }

    /*
     * Value does not dominate anyone
     */
    r.insert(value);
  }

  return r;
}

BasicBlock *DominatorForest::findNearestCommonDominator(BasicBlock *B1,
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

DominatorNode *DominatorForest::findNearestCommonDominator(
    DominatorNode *node1,
    DominatorNode *node2) const {

  /*
   * Helpers to determine whether a node n dominates node2
   */
  auto dominatorsOf2 = this->dominates(node2);
  auto dominates2 = [&](DominatorNode *node) -> bool {
    return dominatorsOf2.find(node) != dominatorsOf2.end();
  };

  /*
   * Traversal of parents of node1 to find common dominator
   */
  DominatorNode *node = node1;
  while (node && !dominates2(node))
    node = node->parent;
  return node;
}

raw_ostream &DominatorForest::print(raw_ostream &stream,
                                    std::string prefixToUse) const {
  for (auto node : nodes) {
    node->print(stream, prefixToUse);
  }
  return stream;
}

} // namespace arcana::noelle
