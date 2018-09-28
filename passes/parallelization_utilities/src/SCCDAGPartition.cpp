#include "SCCDAGPartition.hpp"

using namespace llvm;

SCCDAGSubset::SCCDAGSubset (SCCDAGAttrs *sccdagAttrs, LoopInfoSummary *loopInfo, std::set<SCC *> &sccs)
  : SCCs{sccs} {
  collectSubsetLoopInfo(sccdagAttrs, loopInfo);
}

void SCCDAGSubset::collectSubsetLoopInfo (SCCDAGAttrs *sccdagAttrs, LoopInfoSummary *loopInfo) {

  /*
   * Collect all potentially fully-contained loops in the subset
   */
  std::unordered_map<LoopSummary *, std::set<BasicBlock *>> loopToBBContainedMap;
  for (auto scc : SCCs) {
      for (auto bb : sccdagAttrs->getBasicBlocks(scc)){
          loopToBBContainedMap[loopInfo->bbToLoop[bb]].insert(bb);
      }
  }

  /*
   * Determine which loops are fully contained
   */
  for (auto loopBBs : loopToBBContainedMap) {
      bool fullyContained = true;
      for (auto bb : loopBBs.first->bbs) {
          fullyContained &= loopBBs.second.find(bb) != loopBBs.second.end();
      }
      if (fullyContained) this->loopsContained.insert(loopBBs.first);
  }

  return ;
}

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

raw_ostream &SCCDAGSubset::print (raw_ostream &stream, std::string prefixToUse) {
    return printMinimalSCCs(stream, prefixToUse, this->SCCs);
}

raw_ostream &SCCDAGPartition::print (raw_ostream &stream, std::string prefixToUse) {
  for (auto &subset : this->subsets) {
    subset->print(stream << prefixToUse << "Subset " << getSubsetID(subset) << "\n", prefixToUse);
  }
  return stream;
}

void SCCDAGPartition::initialize (SCCDAG *dag, SCCDAGAttrs *dagInfo, LoopInfoSummary *lInfo) {
  sccDAG = dag;
  sccdagAttrs = dagInfo;
  loopInfo = lInfo;
  subsetCounter = 0;
}

int SCCDAGPartition::getSubsetID (const std::unique_ptr<SCCDAGSubset> &subset) {
  return subsetIDOfSCC(*subset->SCCs.begin());
}

int SCCDAGPartition::subsetIDOfSCC (SCC *scc) {
  auto iter = SCCToSubsetID.find(scc);
  return (iter == SCCToSubsetID.end() ? -1 : iter->second);
}

bool SCCDAGPartition::isValidSubset (int subsetID) {
  return subsetIDToSubset.find(subsetID) != subsetIDToSubset.end();
}

SCCDAGSubset *SCCDAGPartition::subsetOfID (int id) {
  if (!isValidSubset(id)) {
    errs() << "ERROR: Invalid ID for a SCCDAG subset!\n";
    abort();
  }
  return subsetIDToSubset[id];
}

int SCCDAGPartition::addSubset (std::set<SCC *> &sccs) {
  auto newSubset = std::make_unique<SCCDAGSubset>(sccdagAttrs, loopInfo, sccs);
  auto newSubsetPtr = this->subsets.insert(std::move(newSubset)).first->get();
  int newSubsetID = subsetCounter++;
  subsetIDToSubset[newSubsetID] = newSubsetPtr;
  for (auto scc : newSubsetPtr->SCCs) {
    this->SCCToSubsetID[scc] = newSubsetID;
  }
  return newSubsetID;
}

int SCCDAGPartition::addSubset (SCC * scc) {
    std::set<SCC *> sccs = { scc };
    return this->addSubset(sccs);
}

void SCCDAGPartition::removeSubset (int subsetID) {
  if (!isValidSubset(subsetID)) return ;
  auto subset = subsetOfID(subsetID);
  for (auto &p : this->subsets) {
    if (p.get() == subset) {
      for (auto scc : subset->SCCs) {
        auto subsetIDIter = SCCToSubsetID.find(scc);
        if (subsetIDIter == SCCToSubsetID.end()) continue;
        if (subsetIDIter->second != subsetID) continue;
        SCCToSubsetID.erase(scc);
      }
      this->subsets.erase(p);
      subsetIDToSubset.erase(subsetID);
      return;
    }
  }
}

std::set<SCC *> SCCDAGPartition::sccsOfSubsets (int subsetA, int subsetB) {
  std::set<SCC *> allSCCs;
  for (auto scc : subsetOfID(subsetA)->SCCs) allSCCs.insert(scc);
  for (auto scc : subsetOfID(subsetB)->SCCs) allSCCs.insert(scc);
  return allSCCs;
}

int SCCDAGPartition::mergeSubsets (int subsetA, int subsetB) {
  std::set<SCC *> allSCCs = sccsOfSubsets(subsetA, subsetB);
  this->removeSubset(subsetA);
  this->removeSubset(subsetB);
  return this->addSubset(allSCCs);
}

bool SCCDAGPartition::canMergeSubsets (int subAID, int subBID) {
  // Only allow direct ancestors to merge
  auto ancA = this->getAncestorIDs(subAID);
  auto ancB = this->getAncestorIDs(subBID);
  return (!hasAncestor(subAID, subBID) || (ancA.size() == 1 && ancA.find(subBID) != ancA.end()))
    && (!hasAncestor(subBID, subAID) || (ancB.size() == 1 && ancB.find(subAID) != ancB.end()));
}

bool SCCDAGPartition::hasAncestor (int subset, int ancSubset) {
  std::set<int> subsetsSeen;
  std::queue<int> subsetsToVisit;
  subsetsToVisit.push(subset);
  subsetsSeen.insert(subset);
  while (!subsetsToVisit.empty()) {
    auto sub = subsetsToVisit.front();
    subsetsToVisit.pop();
    auto ancestors = this->getAncestorIDs(sub);
    for (auto ancSub : ancestors) {
      if (ancSub == ancSubset) return true;
      if (subsetsSeen.find(ancSub) == subsetsSeen.end()) {
        subsetsToVisit.push(ancSub);
        subsetsSeen.insert(ancSub);
      }
    }
  }
  return false;
}

/*
 * Iterate through all subsets, merging them with those they have memory edges with
 * Then check the SCCDAGPartition until no cycles that may have formed are found
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

std::set<DGNode<SCC> *> SCCDAGPartition::getSCCNodes (int subsetID) {
  std::set<DGNode<SCC> *> sccNodes;
  for (auto scc : subsetOfID(subsetID)->SCCs) sccNodes.insert(sccDAG->fetchNode(scc));
  return sccNodes;    
}

std::set<int> SCCDAGPartition::getDependentIDs (int subsetID) {
  auto sccNodes = getSCCNodes(subsetID);
  std::set<int> subsetIDs = this->getDependentIDs(sccNodes);
  if (subsetIDs.find(subsetID) != subsetIDs.end()) subsetIDs.erase(subsetID);
  return subsetIDs;
}

std::set<int> SCCDAGPartition::getAncestorIDs (int subsetID) {
  auto sccNodes = this->getSCCNodes(subsetID);
  std::set<int> subsetIDs = this->getAncestorIDs(sccNodes);
  if (subsetIDs.find(subsetID) != subsetIDs.end()) subsetIDs.erase(subsetID);
  return subsetIDs;
}

std::set<int> SCCDAGPartition::getDependentIDs (std::set<DGNode<SCC> *> &sccNodes) {
  auto addDependents = [&](std::queue<DGNode<SCC> *> &sccToCheck, DGNode<SCC> *sccNode) -> void {
    for (auto edge : sccNode->getOutgoingEdges()) {
      sccToCheck.push(edge->getIncomingNode());
    }
  };
  return getRelatedIDs(sccNodes, addDependents);
}

std::set<int> SCCDAGPartition::getAncestorIDs (std::set<DGNode<SCC> *> &sccNodes) {
  auto addAncestors = [&](std::queue<DGNode<SCC> *> &sccToCheck, DGNode<SCC> *sccNode) -> void {
    for (auto edge : sccNode->getIncomingEdges()) {
      sccToCheck.push(edge->getOutgoingNode());
    }
  };
  return getRelatedIDs(sccNodes, addAncestors);
}

std::set<int> SCCDAGPartition::getRelatedIDs (std::set<DGNode<SCC> *> &sccNodes,
  std::function<void (std::queue<DGNode<SCC> *> &, DGNode<SCC> *)> addKinFunc
  ) {

  std::set<int> relatedIDs;
  for (auto sccNode : sccNodes) {
    auto selfSubsetID = this->subsetIDOfSCC(sccNode->getT());
    std::queue<DGNode<SCC> *> sccToCheck;
    sccToCheck.push(sccNode);
    while (!sccToCheck.empty()) {
      auto sccNode = sccToCheck.front();
      sccToCheck.pop();

      auto subsetID = this->subsetIDOfSCC(sccNode->getT());
      if (subsetID != -1 && subsetID != selfSubsetID) {
        if (relatedIDs.find(subsetID) == relatedIDs.end()) {
          relatedIDs.insert(subsetID);
        }
        continue;
      }

      addKinFunc(sccToCheck, sccNode);
    }
  }
  return relatedIDs;
}

std::set<int> SCCDAGPartition::getSiblingIDs (int subsetID) {
  std::set<int> ancIDs = this->getAncestorIDs(subsetID);
  std::set<int> depIDs = this->getDependentIDs(subsetID);
  std::set<int> neighborIDs;

  for (auto ancID : ancIDs) {
    auto depSubIDs = this->getDependentIDs(ancID);
    neighborIDs.insert(depSubIDs.begin(), depSubIDs.end());
  }
  for (auto depID : depIDs) {
    auto ancSubIDs = this->getAncestorIDs(depID);
    neighborIDs.insert(ancSubIDs.begin(), ancSubIDs.end());
  }

  if (neighborIDs.find(subsetID) != neighborIDs.end()) neighborIDs.erase(subsetID);
  return neighborIDs;
}

std::set<int> SCCDAGPartition::getSubsetIDsWithNoIncomingEdges () {
  std::set<int> rootSubsetIDs;
  for (auto &subset : subsets) {
    auto subsetID = getSubsetID(subset);
    if (this->getAncestorIDs(subsetID).size() > 0) continue;
    rootSubsetIDs.insert(subsetID);
  }

  return rootSubsetIDs;
}

std::set<int> SCCDAGPartition::nextLevelSubsetIDs (int subsetID) {
  auto subsetIDs = this->getDependentIDs(subsetID);
  std::set<int> nextSubsetIDs;
  for (auto depSubID : subsetIDs) {
    auto prevSubsetIDs = this->getAncestorIDs(depSubID);
    bool noPresentAncestors = true;
    for (auto prevSubID : prevSubsetIDs) {
      if (subsetIDs.find(prevSubID) != subsetIDs.end()) {
        noPresentAncestors = false;
        break;
      }
    }
    if (noPresentAncestors) nextSubsetIDs.insert(depSubID);
  }
  return nextSubsetIDs;
}
