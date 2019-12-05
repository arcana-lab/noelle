/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SCCDAGPartition.hpp"

using namespace llvm;

SCCDAGPartition::SCCDAGPartition (
  SCCDAG *dag,
  SCCDAGAttrs *attrs,
  LoopsSummary *lis,
  std::set<SCCset *> *sets
) : LIS{lis}, sccdag{dag}, dagAttrs{attrs} {
  resetPartition(sets);

  return ;
}

uint64_t SCCDAGPartition::numberOfPartitions (void){
  assert(this->subsets != nullptr);

  return this->subsets->size();
}

void SCCDAGPartition::resetPartition (std::set<SCCset *> *sets) {

  /*
   * Reset the fields.
   */
  subsets = sets;
  this->SCCToSet.clear();

  /*
   * Create the mapping from SCC to its set.
   */
  for (auto subset : *subsets) {
    for (auto scc : *subset) {
      this->SCCToSet[scc] = subset;
    }
  }

  /*
   * Create a program forward ordering of SCCs for debug purposes.
   * This SCC order will be stored in SCCDebugOrder.
   *
   *  - Reset SCCDebugOrder.
   */
  SCCDebugOrder.resize(SCCToSet.size());
  auto count = 0;

  /*
   *  - Fetch the header basic block of the outermost loop. This will be the beginning of the traversal.
   */
  auto topLoop = LIS->getLoopNestingTreeRoot();
  auto bb = topLoop->header;

  /*
   * - Fetch the set of basic blocks that compose the outermost loop. This will be used to avoid iterating over basic blocks outside the outermost loop.
   */
  auto &bbs = topLoop->bbs;

  /*
   *  - Compute SCCDebugOrder.
   */
  std::set<SCC *> sccEncountered;
  std::unordered_map<BasicBlock *, bool> bbsEncountered;
  std::stack<BasicBlock *> todos;
  todos.push(bb);
  while (todos.size() > 0){

    /*
     * Fetch the current basic block.
     */
    auto bb = todos.top();
    todos.pop();

    /*
     * Check if the basic block belongs to the outermost loop.
     */
    if (bbs.find(bb) == bbs.end()){

      /*
       * The basic block does not belong to the outermost loop, so we can skip it.
       */
      continue ;
    }

    /*
     * Check if we have already checked the current basic block.
     */
    if (bbsEncountered.find(bb) != bbsEncountered.end()){

      /*
       * We have already evaluated the current basic block.
       * There is no need to re-evaluate it.
       */
      continue ;
    }
    bbsEncountered[bb] = true;

    /*
     * Consider all instructions of the current basic block.
     */
    for (auto &I : *bb) {

      /*
       * Fetch the SCC that includes the current instruction of the current basic block.
       */
      auto scc = sccdag->sccOfValue(&I);

      /*
       * Check whether the current SCC has been processed already.
       */
      if (SCCToSet.find(scc) == SCCToSet.end()) continue;
      if (sccEncountered.find(scc) != sccEncountered.end()) continue;
      sccEncountered.insert(scc);

      /*
       * Process the new SCC.
       */
      SCCDebugOrder[count] = scc;
      SCCDebugIndex[scc] = count++;
    }

    /*
     * Push on top of the stack all successors of the current basic block.
     */
    for (auto succBB : successors(bb)){
      if (bbsEncountered.find(succBB) == bbsEncountered.end()){
        todos.push(succBB);
      }
    }
  }

  /*
   *  - Check everything is correct.
   */
  if (SCCDebugIndex.size() != SCCToSet.size()) {
    errs() << "ERROR: Mismatch # of SCC encountered (in program forward order traversal): "
      << SCCDebugIndex.size() << " versus total # of SCC in subsets: "
      << SCCToSet.size() << "\n";
    assert(false && "SCCDAGPartition::resetPartition");
  }

  /*
   * Ensure the initial specified configuration has no cycles.
   */
  if (hasCycle()) {
    mergeCycles();
  }

  /*
   * Determine a depth ordering of subsets based on their parent-children graph.
   */
  resetSubsetGraph();
  orderSubsets();

  return ;
}

SCCset *SCCDAGPartition::mergePairAndCycles (SCCset *subsetA, SCCset *subsetB) {
  auto merged = mergePair(subsetA, subsetB, /*doReorder=*/false);

  if (hasCycle()) {
    mergeCycles();
    resetSubsetGraph();
  }
  orderSubsets();
  return merged;
}

SCCset *SCCDAGPartition::mergePair (SCCset *subsetA, SCCset *subsetB, bool doReorder) {
  auto mergedSubset = new SCCset(subsetA->begin(), subsetA->end());
  mergedSubset->insert(subsetB->begin(), subsetB->end());
  subsets->erase(subsetA);
  subsets->erase(subsetB);
  subsets->insert(mergedSubset);
  for (auto scc : *mergedSubset) SCCToSet[scc] = mergedSubset;

  // printNodeInGraph(errs(), "DEBUG: ", subsetA);
  // printNodeInGraph(errs(), "DEBUG: ", subsetB);

  auto transferRelations = [&](SCCset *from, SCCset *to) -> void {
    for (auto parent : parentSubsets[from]) {
      parentSubsets[to].insert(parent);
      childrenSubsets[parent].insert(to);
      childrenSubsets[parent].erase(from);
    }
    for (auto child : childrenSubsets[from]) {
      childrenSubsets[to].insert(child);
      parentSubsets[child].insert(to);
      parentSubsets[child].erase(from);
    }
    parentSubsets.erase(from);
    childrenSubsets.erase(from);
  };
  transferRelations(subsetA, mergedSubset);
  transferRelations(subsetB, mergedSubset);

  roots.erase(subsetA);
  roots.erase(subsetB);
  parentSubsets[mergedSubset].erase(mergedSubset);
  if (parentSubsets[mergedSubset].size() == 0) {
    parentSubsets.erase(mergedSubset);
    roots.insert(mergedSubset);
  }
  childrenSubsets[mergedSubset].erase(mergedSubset);
  if (childrenSubsets[mergedSubset].size() == 0) {
    childrenSubsets.erase(mergedSubset);
  }

  // printNodeInGraph(errs(), "DEBUG: ", mergedSubset);

  if (doReorder) orderSubsets();
  return mergedSubset;
}

/*
 * Iterate subsets, merging subsets that share a memory edge
 * Then, remove any cycles that were formed in the partition via further merging
 */
bool SCCDAGPartition::mergeAlongMemoryEdges () {
  auto fetchAlongMemoryEdge = [&](SCCset *subset) -> SCCset * {
    for (auto scc : *subset) {
      for (auto edge : sccdag->fetchNode(scc)->getOutgoingEdges()) {
        bool hasMemEdge = false;
        for (auto subEdge : edge->getSubEdges()) {
          hasMemEdge |= subEdge->isMemoryDependence();
        }
        if (!hasMemEdge) continue;
        auto otherSubset = SCCToSet[edge->getIncomingT()];
        if (otherSubset == subset) continue;
        return otherSubset;
      }
    }
    return nullptr;
  };

  std::queue<SCCset *> subToCheck;
  for (auto root : roots) subToCheck.push(root);
  while (!subToCheck.empty()) {
    auto subset = subToCheck.front();
    subToCheck.pop();
    if (childrenSubsets.find(subset) == childrenSubsets.end()) continue;

    /*
     * Find the first subset to merge with
     */
    SCCset *mergeWith = fetchAlongMemoryEdge(subset);
    if (mergeWith) {
      auto mergedSub = mergePairAndCycles(subset, mergeWith);
      return true;
    }
    for (auto child : childrenSubsets[subset]) subToCheck.push(child);
  }

  return false;
}

std::set<SCCset *> *SCCDAGPartition::getParents(SCCset *subset) {
  if (roots.find(subset) != roots.end()) return nullptr;
  return &parentSubsets[subset];
}

std::set<SCCset *> *SCCDAGPartition::getChildren(SCCset *subset) {
  if (childrenSubsets.find(subset) == childrenSubsets.end()) return nullptr;
  return &childrenSubsets[subset];
}

raw_ostream &SCCDAGPartition::print (raw_ostream &stream, std::string prefix) {
  printSCCIndices(stream, prefix);
  printGraph(stream, prefix);
  return stream;
}

raw_ostream &SCCDAGPartition::printSCCIndices (raw_ostream &stream, std::string prefix) {
  for (auto i = 0; i < SCCDebugOrder.size(); ++i) {
    stream << prefix << "SCC: " << i << " Internal nodes:\n";
    for (auto nodePair : SCCDebugOrder[i]->internalNodePairs()) {
      nodePair.first->print(stream << prefix << "  ");
      stream << "\n";
    }
  }
  return stream;
}

std::string SCCDAGPartition::subsetStr (SCCset *subset) {
  std::string text = "Set: ";
  raw_string_ostream str(text);
  for (auto scc : *subset) str << " " << SCCDebugIndex[scc];
  str.str();
  return text;
};

raw_ostream &SCCDAGPartition::printNodeInGraph (raw_ostream &stream, std::string prefix, SCCset *subset) {
  stream << prefix << subsetStr(subset) << "\n" << prefix << " Children: ";
  if (childrenSubsets.find(subset) != childrenSubsets.end()) {
    for (auto child : childrenSubsets[subset]) stream << subsetStr(child) << "; ";
  }
  stream << "\n" << prefix << " Parents: ";
  if (parentSubsets.find(subset) != parentSubsets.end()) {
    for (auto parent : parentSubsets[subset]) stream << subsetStr(parent) << "; ";
  }
  stream << "\n";
  return stream;
}

raw_ostream &SCCDAGPartition::printGraph (raw_ostream &stream, std::string prefix) {
  std::set<SCCset *> subsetsEncountered;
  for (auto scc : SCCDebugOrder) {
    auto subset = SCCToSet[scc];
    if (subsetsEncountered.find(subset) != subsetsEncountered.end()) continue;
    subsetsEncountered.insert(subset);
    printNodeInGraph(stream, prefix, subset);
  }
  return stream;
}

void SCCDAGPartition::resetSubsetGraph () {
  roots.clear();
  parentSubsets.clear();
  childrenSubsets.clear();
  collectSubsetGraph();
}

void SCCDAGPartition::collectSubsetGraph () {
  for (auto subset : *subsets) {

    /*
     * NOTE(angelo): Only scc within subsets are considered as parents
     */
    std::set<SCCset *> parents;
    for (auto scc : *subset) {
      for (auto parentSCC : dagAttrs->parentsViaClones[scc]) {
        auto subsetIter = SCCToSet.find(parentSCC);
        if (subsetIter != SCCToSet.end()) {
          parents.insert(subsetIter->second);
        }
      }
    }

    parents.erase(subset);
    if (parents.size() == 0) roots.insert(subset);
    for (auto parent : parents) {
      parentSubsets[subset].insert(parent);
      childrenSubsets[parent].insert(subset);
    }
  }
}

bool SCCDAGPartition::hasCycle () {
  if (roots.size() == 0) return true;

  std::queue<SCCset *> subsetsToCheck;
  for (auto root : roots) subsetsToCheck.push(root);
  std::set<SCCset *> encountered(roots.begin(), roots.end());
  while (!subsetsToCheck.empty()) {
    auto subset = subsetsToCheck.front();
    subsetsToCheck.pop();
    if (childrenSubsets.find(subset) == childrenSubsets.end()) continue;

    /*
     * Confirm all parents have been encountered before traversing further
     */
    if (parentSubsets.find(subset) != parentSubsets.end()) {
      bool parentsEncountered = true;
      for (auto parent : parentSubsets[subset]) {
        parentsEncountered &= encountered.find(parent) != encountered.end();
      }
      if (!parentsEncountered) continue;
    }

    for (auto child : childrenSubsets[subset]) {
      if (encountered.find(child) == encountered.end()) {
        subsetsToCheck.push(child);
      }
    }
  }

  /*
   * If not all subsets were encountered, at some point a cycle was encountered
   *  that prevented a subset-depth traversal from completing
   */
  return encountered.size() != subsets->size();
}

/*
 * NOTE(angelo): Depth order is the longest path to that subset
 * from subsets without parents
 */
void SCCDAGPartition::orderSubsets () {
  subsetDepths.clear();
  for (auto subset : roots) subsetDepths[subset] = 0;
  std::queue<SCCset *> subsetsToCheck;
  for (auto root : roots) subsetsToCheck.push(root);
  while (!subsetsToCheck.empty()) {
    auto subset = subsetsToCheck.front();
    subsetsToCheck.pop();

    auto childDepth = subsetDepths[subset] + 1;
    if (childrenSubsets.find(subset) == childrenSubsets.end()) continue;
    for (auto child : childrenSubsets[subset]) {
      subsetsToCheck.push(child);
      subsetDepths[child] = childDepth;
    }
  }

  depthOrderedSubsets.resize(subsets->size());
  assert(subsetDepths.size() == subsets->size());
  std::vector<std::set<SCCset *>> depthBuckets(subsets->size());
  for (auto subsetDepth : subsetDepths) {
    depthBuckets[subsetDepth.second].insert(subsetDepth.first);
  }
  auto count = 0;
  for (auto bucket : depthBuckets) {
    for (auto subset : bucket) {
      depthOrderedSubsets[count++] = subset;
    }
  }
}

/*
 * As long as the pair are a direct parent/child or are not either's
 * ancestor, no cycle is formed
 */
bool SCCDAGPartition::mergeYieldsCycle (SCCset *subsetA, SCCset *subsetB) {
  auto isAncestor = [&](SCCset *parent, SCCset *child) -> bool {
    if (roots.find(child) != roots.end()) return false;
    std::queue<SCCset *> subToCheck;
    for (auto par : parentSubsets[child]) subToCheck.push(par);
    while (!subToCheck.empty()) {
      auto subset = subToCheck.front();
      subToCheck.pop();
      if (roots.find(subset) != roots.end()) continue;
      for (auto par : parentSubsets[subset]) {
        if (par == parent) return true;
        subToCheck.push(par);
      }
    }
    return false;
  };

  return isAncestor(subsetA, subsetB) || isAncestor(subsetB, subsetA);
}

/*
 * From each root node, recursively traverse all edges, tracking the path taken.
 * When a cycle is encountered, recursively merge away the chain
 * Do this until all top level nodes traversed all paths and found no cycles
 */
bool SCCDAGPartition::mergeCycles () {
  std::set<SCCset *> toCheck(roots.begin(), roots.end());
  if (toCheck.size() == 0) toCheck.insert(subsets->begin(), subsets->end());
  bool merged = false;
  for (auto sub : toCheck) {
    std::vector<SCCset *> path = { sub };
    merged |= this->traverseAndMerge(path);
  }
  return merged;
}

bool SCCDAGPartition::traverseAndMerge (std::vector<SCCset *> &path) {
  auto subset = path.back();
  if (childrenSubsets.find(subset) == childrenSubsets.end()) return false;

  bool didMerge = false;
  bool merged = true;
  while (merged) {
    merged = false;

    for (auto child : childrenSubsets[subset]) {
      auto subIter = std::find(path.begin(), path.end(), child);

      /*
       * If child doesn't form a cycle in our path, recursively try merging
       * Else, merge the whole cycle contained in our path and return
       */
      if (subIter == path.end()) {
        std::vector<SCCset *> nextPath(path.begin(), path.end());
        nextPath.push_back(child);
        bool nextDidMerge = traverseAndMerge(nextPath);

        /*
         * Current subset was merged away
         */
        if (nextPath.size() <= path.size()) {
          path.erase(path.begin() + nextPath.size(), path.end());
          path[path.size() - 1] = nextPath[path.size() - 1];
          return true;
        }

        /*
         * Dependent was merged; restart scan through dependents
         */
        if (nextDidMerge) {
          didMerge = merged = true;
          break;
        }
      } else {
        auto newEndIter = subIter;
        SCCset *mergedCycle = *subIter;
        while (++subIter != path.end()) {
          mergedCycle = mergePair(mergedCycle, *subIter);
        }
        path.erase(newEndIter, path.end());
        path.push_back(mergedCycle);
        return true;
      }
    }
  }

  return didMerge;
}
