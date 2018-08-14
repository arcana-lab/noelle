#include "SCCDAGPartition.hpp"

using namespace llvm;

SCCDAGSubset::SCCDAGSubset (SCCDAGAttrs *sccdagAttrs, LoopInfoSummary *loopInfo, std::set<SCC *> &sccs)
  : SCCs{sccs} {
  collectSubsetLoopInfo(sccdagAttrs, loopInfo);
}

SCCDAGSubset::SCCDAGSubset (SCCDAGAttrs *sccdagAttrs, LoopInfoSummary *loopInfo, SCCDAGSubset *subsetA, SCCDAGSubset *subsetB) {
  for (auto scc : subsetA->SCCs) this->SCCs.insert(scc);
  for (auto scc : subsetB->SCCs) this->SCCs.insert(scc);
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
    for (auto &removableSCC : sccs) {
        stream << prefixToUse << "Internal nodes: " << "\n";
        for (auto nodePair : removableSCC->internalNodePairs()) {
            nodePair.first->print(stream << prefixToUse << "\t");
            stream << "\n";
        }
    }
    return stream;
}

raw_ostream &SCCDAGSubset::print (raw_ostream &stream, std::string prefixToUse) {
    return printMinimalSCCs(stream, prefixToUse, this->SCCs);
}

SCCDAGSubset *SCCDAGPartition::addSubset (SCC * scc) {
    std::set<SCC *> sccs = { scc };
    return this->addSubset(sccs);
}

SCCDAGSubset *SCCDAGPartition::addSubset (std::set<SCC *> &sccs) {
    auto subset = std::make_unique<SCCDAGSubset>(sccdagAttrs, loopInfo, sccs);
    auto subsetPtr = this->subsets.insert(std::move(subset)).first->get();
    this->manageAddedSubsetInfo(subsetPtr);
    return subsetPtr; 
}

void SCCDAGPartition::initialize (SCCDAG *dag, SCCDAGAttrs *dagInfo, LoopInfoSummary *lInfo) {
    sccDAG = dag;
    sccdagAttrs = dagInfo;
    loopInfo = lInfo;
}

void SCCDAGPartition::removeSubset (SCCDAGSubset *subset) {
    for (auto &p : this->subsets) {
        if (p.get() == subset) {
            this->subsets.erase(p);
            return;
        }
    }
}

SCCDAGSubset *SCCDAGPartition::mergeSubsets (SCCDAGSubset *subsetA, SCCDAGSubset *subsetB) {
    auto subset = std::make_unique<SCCDAGSubset>(sccdagAttrs, loopInfo, subsetA, subsetB);
    auto newSubset = this->subsets.insert(std::move(subset)).first->get();

    this->removeSubset(subsetA);
    this->removeSubset(subsetB);
    this->manageAddedSubsetInfo(newSubset);
    return newSubset;
}

SCCDAGSubset *SCCDAGPartition::demoMergeSubsets (SCCDAGSubset *subsetA, SCCDAGSubset *subsetB) {
    return new SCCDAGSubset(sccdagAttrs, loopInfo, subsetA, subsetB);
}

bool SCCDAGPartition::canMergeSubsets (SCCDAGSubset *subsetA, SCCDAGSubset *subsetB) {
    std::set<SCC *> outgoingToB;
    for (auto scc : subsetB->SCCs) {
        for (auto edge : sccDAG->fetchNode(scc)->getIncomingEdges()) {
            outgoingToB.insert(edge->getOutgoingT());
        }
    }

    /*
     * Check that no cycle would form by merging the subsets
     */
    for (auto scc : subsetA->SCCs) {
        for (auto edge : sccDAG->fetchNode(scc)->getOutgoingEdges()) {
            if (subsetA->SCCs.find(edge->getIncomingT()) != subsetA->SCCs.end()) continue;
            if (outgoingToB.find(edge->getIncomingT()) != outgoingToB.end()) return false;
        }
    }
    return true;
}

void SCCDAGPartition::manageAddedSubsetInfo (SCCDAGSubset *subset) {
    for (auto scc : subset->SCCs) this->fromSCCToSubset[scc] = subset;
}

/*
 * Iterate through all subsets, merging them with those they have memory edges with
 * Then check the SCCDAGPartition until no cycles that may have formed are found
 */
void SCCDAGPartition::mergeSubsetsRequiringMemSync () {
  /*
   * Track alive subsets to ignore merged ones still in the queue
   */
  std::set<SCCDAGSubset *> currentSubsets;
  for (auto &subset : this->subsets) currentSubsets.insert(subset.get());

  /*
   * Start traversal from the root of the partition dag
   */
  std::queue<SCCDAGSubset *> subToCheck;
  auto rootSubs = this->topLevelSubsets();
  for (auto &sub : rootSubs) subToCheck.push(sub);

  while (!subToCheck.empty()) {
    auto subset = subToCheck.front();
    subToCheck.pop();

    if (currentSubsets.find(subset) == currentSubsets.end()) {
      continue;
    }

    SCCDAGSubset *mergeSubset = nullptr;

    /*
     * Find the first subset to merge with; end search there
     */
    auto sccNodes = this->getSCCNodes(subset);
    for (auto sccNode : sccNodes) {
      for (auto edge : sccNode->getOutgoingEdges()) {
        bool hasMemEdge = false;
        for (auto subEdge : edge->getSubEdges()) {
          hasMemEdge |= subEdge->isMemoryDependence();
        }
        if (!hasMemEdge) continue;
        auto otherSubset = this->subsetOf(edge->getIncomingT());
        if (otherSubset == subset) continue;
        mergeSubset = otherSubset;
        break;
      }
      if (mergeSubset) break;
    }
    
    /*
     * Check the merged subset for more memory edges, or its dependents
     */
    if (mergeSubset) {
      bool canMergeWithoutMakingCycle = this->canMergeSubsets(subset, mergeSubset);
      auto mergedSub = this->mergeSubsets(subset, mergeSubset);
      subToCheck.push(mergedSub);

      if (canMergeWithoutMakingCycle) {
        currentSubsets.erase(subset);
        currentSubsets.erase(mergeSubset);
        currentSubsets.insert(mergedSub);
      } else {
        this->mergeSubsetsFormingCycles();
        this->mergeSubsetsRequiringMemSync();
        return;
      }
    } else {
      auto dependents = this->getDependents(subset);
      for (auto depSub : dependents) subToCheck.push(depSub);
    }
  }

  /*
   * Remove any potential cycles created from merging
   */
  this->mergeSubsetsFormingCycles();
}

void SCCDAGPartition::mergeSubsetsFormingCycles () {
  // From each root node, recursively traverse all edges tracking path
  // When a cycle is encountered, return up the chain, have the top merge subsets in the cyclical path
  // Do this until all top level nodes traversed all paths and found no cycles

  /*
   * Start traversal from the root of the partition dag
   */
  auto rootSubs = this->topLevelSubsets();
  for (auto &sub : rootSubs) {
    std::vector<SCCDAGSubset *> path = { sub };
    this->traverseAndCheckToMerge(path);
  }
}

SCCDAGSubset *SCCDAGPartition::traverseAndCheckToMerge (std::vector<SCCDAGSubset *> &path) {
  auto depSubs = this->getDependents(path.back());
  bool merged = true;
  while (merged) {
    merged = false;
    for (auto sub : depSubs) {
      auto subIter = std::find(path.begin(), path.end(), sub);

      /*
       * If dependent doesn't form a cycle in our path, recursively try merging
       * Else, merge the whole cycle contained in our path and return
       */
      if (subIter == path.end()) {
        std::vector<SCCDAGSubset *> nextPath(path.begin(), path.end());
        nextPath.push_back(sub);
        auto mergedSub = traverseAndCheckToMerge(nextPath);

        /*
         * Current subset was merged away, so return the newly merged subset
         */
        if (nextPath.size() <= path.size()) {
          path.erase(path.begin() + nextPath.size(), path.end());
          path[path.size() - 1] = mergedSub;
          return mergedSub;
        }

        /*
         * Dependent was merged; restart scan through dependents
         */
        if (mergedSub) {
          merged = true;
          break;
        }
      } else {
        SCCDAGSubset *mergedSub = *(subIter++);
        while (subIter != path.end()) {
          auto mergableSub = *subIter;
          mergedSub = this->mergeSubsets(mergedSub, mergableSub);
          subIter++;
        }
        return mergedSub;
      }
    }
  }

  /*
   * We did not merge with any other subset
   */
  return nullptr;
}

SCCDAGSubset *SCCDAGPartition::subsetOf (SCC *scc) {
    auto iter = this->fromSCCToSubset.find(scc);
    return (iter == this->fromSCCToSubset.end() ? nullptr : iter->second);
}

bool SCCDAGPartition::isRemovable (SCC *scc) {
    return (this->removableNodes.find(scc) != this->removableNodes.end());
}

int SCCDAGPartition::numEdgesBetween (SCCDAGSubset *subsetA, SCCDAGSubset *subsetB) {
    int edgeCount = 0;
    for (auto scc : subsetA->SCCs) {
        for (auto edge : sccDAG->fetchNode(scc)->getOutgoingEdges()) {
            if (subsetB->SCCs.find(edge->getIncomingT()) != subsetB->SCCs.end()) {
                edgeCount++;
            }
        }
    }
    return edgeCount;
}

std::set<DGNode<SCC> *> SCCDAGPartition::getSCCNodes (SCCDAGSubset *subset) {
    std::set<DGNode<SCC> *> sccNodes;
    for (auto scc : subset->SCCs) sccNodes.insert(this->sccDAG->fetchNode(scc));
    return sccNodes;    
}

std::set<SCCDAGSubset *> SCCDAGPartition::getDependents (SCCDAGSubset *subset) {
    auto sccNodes = this->getSCCNodes(subset);
    std::set<SCCDAGSubset *> subsets = this->getDependents(sccNodes);
    if (subsets.find(subset) != subsets.end()) subsets.erase(subset);
    return subsets;
}

std::set<SCCDAGSubset *> SCCDAGPartition::getAncestors (SCCDAGSubset *subset) {
    auto sccNodes = this->getSCCNodes(subset);
    std::set<SCCDAGSubset *> subsets = this->getAncestors(sccNodes);
    if (subsets.find(subset) != subsets.end()) subsets.erase(subset);
    return subsets;
}

std::set<SCCDAGSubset *> SCCDAGPartition::getDependents (std::set<DGNode<SCC> *> &sccNodes) {
    auto addDependents = [&](std::queue<DGNode<SCC> *> &sccToCheck, DGNode<SCC> *sccNode) -> void {
        for (auto edge : sccNode->getOutgoingEdges()) {
            sccToCheck.push(edge->getIncomingNode());
        }
    };
    return getRelated(sccNodes, addDependents);
}

std::set<SCCDAGSubset *> SCCDAGPartition::getAncestors (std::set<DGNode<SCC> *> &sccNodes) {
    auto addAncestors = [&](std::queue<DGNode<SCC> *> &sccToCheck, DGNode<SCC> *sccNode) -> void {
        for (auto edge : sccNode->getIncomingEdges()) {
            sccToCheck.push(edge->getOutgoingNode());
        }
    };
    return getRelated(sccNodes, addAncestors);
}

std::set<SCCDAGSubset *> SCCDAGPartition::getRelated (std::set<DGNode<SCC> *> &sccNodes,
    std::function<void (std::queue<DGNode<SCC> *> &, DGNode<SCC> *)> addKinFunc) {

    std::set<SCCDAGSubset *> related;
    for (auto sccNode : sccNodes) {
        auto selfSubset = this->subsetOf(sccNode->getT());
        std::queue<DGNode<SCC> *> sccToCheck;
        sccToCheck.push(sccNode);
        while (!sccToCheck.empty()) {
            auto sccNode = sccToCheck.front();
            sccToCheck.pop();

            auto subset = this->subsetOf(sccNode->getT());
            if (subset && subset != selfSubset) {
                if (related.find(subset) == related.end()) {
                    related.insert(subset);
                }
                continue;
            }

            addKinFunc(sccToCheck, sccNode);
        }
    }
    return related;
}

std::set<SCCDAGSubset *> SCCDAGPartition::getCousins (SCCDAGSubset *subset) {
    auto sccNodes = this->getSCCNodes(subset);
    std::set<SCCDAGSubset *> subsets = this->getAncestors(sccNodes);
    if (subsets.find(subset) != subsets.end()) subsets.erase(subset);
    
    std::set<SCCDAGSubset *> neighbors;
    for (auto otherSubset : subsets) {
        auto partSCCNodes = this->getSCCNodes(otherSubset);
        auto otherParts = this->getDependents(partSCCNodes);
        if (otherParts.find(otherSubset) != otherParts.end()) otherParts.erase(otherSubset);
        if (otherParts.find(subset) != otherParts.end()) otherParts.erase(subset);
        neighbors.insert(otherParts.begin(), otherParts.end());
    }
    return neighbors;
}

std::set<SCCDAGSubset *> SCCDAGPartition::topLevelSubsets () {
    std::set<SCCDAGSubset *> topLevelSubsets;
    auto topLevelNodes = sccDAG->getTopLevelNodes();
    for (auto node : topLevelNodes) {
        auto subset = this->subsetOf(node->getT());
        if (subset) topLevelSubsets.insert(subset);
    }

    /*
     * Should the top level nodes be removable, grab their descendants which belong to subsets
     */
    if (topLevelSubsets.size() == 0) {
        topLevelSubsets = this->getDependents(topLevelNodes);
    }

    std::set<SCCDAGSubset *> rootSubsets;
    for (auto subset : topLevelSubsets) {
        if (this->getAncestors(subset).size() > 0) continue;
        rootSubsets.insert(subset);
    }

    return rootSubsets;
}

std::set<SCCDAGSubset *> SCCDAGPartition::nextLevelSubsets (SCCDAGSubset *subset) {
    auto subsets = this->getDependents(subset);
    std::set<SCCDAGSubset *> nextSubsets;
    for (auto depSub : subsets) {
        auto prevSubsets = this->getAncestors(depSub);
        bool noPresentAncestors = true;
        for (auto prevSub : prevSubsets) {
            if (subsets.find(prevSub) != subsets.end()) {
                noPresentAncestors = false;
                break;
            }
        }
        if (noPresentAncestors) nextSubsets.insert(depSub);
    }
    return nextSubsets;
}

raw_ostream &SCCDAGPartition::print (raw_ostream &stream, std::string prefixToUse) {
    for (auto &subset : this->subsets) {
        subset->print(stream << prefixToUse << "Subset:\n", prefixToUse);
    }
    return printMinimalSCCs(stream << prefixToUse << "Removable nodes:\n", prefixToUse, this->removableNodes);
}
