#include "SCCDAGPartition.hpp"

using namespace llvm;

raw_ostream &printMinimalSCCs (raw_ostream &stream, std::string prefixToUse, std::set<SCC *> &sccs) {
  if (sccs.size() == 0) return stream;
  for (auto scc : sccs) {
    stream << prefixToUse << "Internal nodes: " << "\n";
    for (auto nodePair : scc->internalNodePairs()) {
      nodePair.first->print(stream << prefixToUse << "\t");
      stream << "\n";
    }
  }
  return stream;
}

raw_ostream &SCCDAGPartition::print (raw_ostream &stream, std::string prefixToUse) {
  for (auto &subset : this->subsets) {
    subset->print(stream << prefixToUse << "Subset " << getSubsetID(subset) << "\n", prefixToUse);
  }
  return stream;
}

void SCCDAGPartition::initialize (SCCDAG *dag) {
  sccDAG = dag;
}

void SCCDAGPartition::addSubset (SCCset *subset) {
  subsets.insert(subset);
  for (auto scc : *subset) {
    SCCToSubset[scc] = subset;
  }
}

void SCCDAGPartition::createSubset (SCCset *sccs) {
  auto subset = new std::set<SCC *>(sccs.begin(), sccs.end());
  addSubset(subset);
  subsetGraphBuilt = false;
}

void SCCDAGPartition::createSubset (SCC * scc) {
  auto subset = new SCCset();
  subset.insert(scc);
  addSubset(subset);
  subsetGraphBuilt = false;
}

void SCCDAGPartition::validateSubsetOrder () {
  /* 
   * NOTE(angelo): The subset graph is maintained by partition API calls,
   *  so it is built only once
   */
  if (!subsetGraphBuilt) {
    collectSubsetGraph();
    subsetGraphBuilt = true;
  }
  orderSubsets();
}

void collectSubsetGraph () {
  auto addIncomingNodes = [](std::queue<DGNode<SCC> *> &queue, DGNode<SCC> &node) -> void {
    std::set<DGNode<SCC> *> nodes;
    for (auto edge : node->getIncomingEdges() {
      nodes.insert(edge->getOutgoingNode());
    }
    for (auto node : nodes) queue.insert(node);
  };

  for (auto subset : subsets) {
    std::set<std::set<SCC *> *> parents;

    for (auto scc : subset) {
      auto sccNode = sccDAG->fetchNode(scc);
      std::queue<DGNode<SCC> *> nodesToCheck;

      while (!sccToCheck.empty()) {
        auto node = nodesToCheck.front();
        nodesToCheck.pop();

        /*
         * NOTE(angelo): If the SCC does not belong to a partition, it is removable.
         * We then consider the removable SCC's parents to be our parents
         */
        auto subsetIter = SCCToSet.find(node->getT());
        if (subsetIter == subsetIter.end()) {
          addIncomingNodes(nodesToCheck, node);
        } else {
          parents.insert(*subsetIter);
        }
      }
    }

    if (parents.size() == 0) roots.insert(subset);
    for (auto parent : parents) {
      parentSubsets[subset].insert(parent);
      childrenSubsets[parent].insert(subset);
    }
  }
}

/*
 * NOTE(angelo): Depth order is the longest path to that subset
 * from subsets without parents
 */
void orderSubsets () {
  for (auto subset : roots) subsetDepths[subset] = 0;
  std::queue<std::set<SCC *> *> subsetsToCheck(roots.begin(), roots.end());
  while (!subsetsToCheck.empty()) {
    auto subset = subsetsToCheck.front();
    subsetsToCheck.pop();

    auto childDepth = subsetDepths[subset] + 1;
    for (auto child : childrenSubsets[subset]) {
      subsetsToCheck.push(child);
      subsetDepths[child] = childDepth;
    }
  }

  depthOrderedSubsets.resize(subsets.size());
  for (auto subsetDepth : subsetDepths) {
    depthOrderedSubsets[subsetDepth.second] = subsetDepth.first;
  }
}

bool SCCDAGPartition::hasCycle () {
  std::set<std::set<SCC *> *> encountered;
  std::queue<std::set<SCC *> *> subsetsToCheck(roots.begin(), roots.end());
  while (!subsetsToCheck.empty()) {
    auto subset = subsetsToCheck.front();
    subsetsToCheck.pop();

    bool parentsEncountered = true;
    for (auto parent : parentSubsets[subset]) {
      parentsEncountered &= encountered.find(parent) != encountered.end();
    }
    if (!parentsEncountered) continue;
    for (auto child : childrenSubsets[subset]) {
      subsetsToCheck.push(child);
    }
  }
  return encountered.size() != subsets.size();
}

SCCset *SCCDAGPartition::mergeAndSquashCycles (SCCset &subsetA, SCCset &subsetB) {
  auto act = new PartitionTransaction();
  act->oldSets.insert(&subsetA);
  act->oldSets.insert(&subsetB);

  auto mergedSubset = new SCCset(subsetA.begin(), subsetA.end());
  mergedSubset->insert(subsetB.begin(), subsetB.end());
  act->newSets.insert(mergedSubset);

  subsets.erase(&subsetA);
  subsets.erase(&subsetB);
  this->addSubset(allSCCs);

  validateSubsetOrder();
}

/*
 * Iterate subsets, merging subsets that share a memory edge
 * Then, remove any cycles that were formed in the partition via further merging
 */
void SCCDAGPartition::mergeSubsetsRequiringMemSync () {

  /*
   * Start traversal from the root of the partition dag
   */
  std::queue<int> subIDToCheck;
  auto rootSubIDs = this->getSubsetIDsWithNoIncomingEdges();
  for (auto subID : rootSubIDs) subIDToCheck.push(subID);

  while (!subIDToCheck.empty()) {
    auto subsetID = subIDToCheck.front();
    subIDToCheck.pop();

    if (!isValidSubset(subsetID)) continue ;

    /*
     * Find the first subset to merge with; end search there
     */
    int mergeWithID = -1;
    auto sccNodes = this->getSCCNodes(subsetID);
    for (auto sccNode : sccNodes) {
      for (auto edge : sccNode->getOutgoingEdges()) {
        bool hasMemEdge = false;
        for (auto subEdge : edge->getSubEdges()) {
          hasMemEdge |= subEdge->isMemoryDependence();
        }
        if (!hasMemEdge) continue;
        auto otherSubsetID = subsetIDOfSCC(edge->getIncomingT());
        if (otherSubsetID == subsetID) continue;
        mergeWithID = otherSubsetID;
        break;
      }
      if (mergeWithID != -1) break;
    }
    
    /*
     * Check the merged subset for more memory edges, or its dependents
     */
    if (mergeWithID != -1) {
      bool canMergeWithoutMakingCycle = this->canMergeSubsets(subsetID, mergeWithID);
      auto mergedSubID = this->mergeSubsets(subsetID, mergeWithID);
      subIDToCheck.push(mergedSubID);

      if (!canMergeWithoutMakingCycle) {
        this->mergeSubsetsFormingCycles();
        this->mergeSubsetsRequiringMemSync();
        return;
      }
    } else {
      auto dependentIDs = this->getDependentIDs(subsetID);
      for (auto depSubID : dependentIDs) subIDToCheck.push(depSubID);
    }
  }

  /*
   * Remove any potential cycles created from merging
   */
  // NOTE(angelo): this shouldn't be needed because we do this upon every merge?
  // this->mergeSubsetsFormingCycles();
}

void SCCDAGPartition::mergeSubsetsFormingCycles () {

  /*
   * From each root node, recursively traverse all edges tracking path
   * When a cycle is encountered, return up the chain, have the top merge subsets in the cyclical path
   * Do this until all top level nodes traversed all paths and found no cycles
   */
  auto rootSubIDs = this->getSubsetIDsWithNoIncomingEdges();
  for (auto subID : rootSubIDs) {
    std::vector<int> path = { subID };
    this->traverseAndCheckToMerge(path);
  }
}

int SCCDAGPartition::traverseAndCheckToMerge (std::vector<int> &path) {
  auto subsetID = path.back();
  bool merged = true;
  while (merged) {
    merged = false;

    auto subset = subsetOfID(subsetID);
    auto depSubIDs = this->getDependentIDs(subsetID);
    for (auto subID : depSubIDs) {
      if (!isValidSubset(subID)) continue ;
      auto subIter = std::find(path.begin(), path.end(), subID);

      /*
       * If dependent doesn't form a cycle in our path, recursively try merging
       * Else, merge the whole cycle contained in our path and return
       */
      if (subIter == path.end()) {
        std::vector<int> nextPath(path.begin(), path.end());
        nextPath.push_back(subID);
        auto mergedSubID = traverseAndCheckToMerge(nextPath);

        /*
         * Current subset was merged away, so return the newly merged subset
         */
        if (nextPath.size() <= path.size()) {
          path.erase(path.begin() + nextPath.size(), path.end());
          path[path.size() - 1] = mergedSubID;
          return mergedSubID;
        }

        /*
         * Dependent was merged; restart scan through dependents
         */
        if (mergedSubID != -1) {
          merged = true;
          break;
        }
      } else {
        auto newPathEndIter = subIter;
        int mergedSubID = *(subIter++);
        while (subIter != path.end()) {
          int mergableSubID = *subIter;
          mergedSubID = this->mergeSubsets(mergedSubID, mergableSubID);
          subIter++;
        }
        path.erase(newPathEndIter, path.end());
        path.push_back(mergedSubID);
        return mergedSubID;
      }
    }
  }

  /*
   * We did not merge with any other subset
   */
  return -1;
}
