/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni, Brian Homerding
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/SCCDAGPartition.hpp"

using namespace llvm;
using namespace llvm::noelle;

SCCDAGPartition::SCCDAGPartition (
  SCCDAG *sccdag,
  std::unordered_set<SCCSet *> initialSets,
  std::unordered_map<SCC *, std::unordered_set<SCC *>> sccToParentsMap
) : sccdag{sccdag}, sccToSetMap{} {

  /*
   * Create nodes for each set and relate their member SCCs to that set
   */
  for (auto initialSet : initialSets) {
    auto set = new SCCSet();
    set->sccs = initialSet->sccs;
    this->addNode(set, /*inclusion=*/true);

    for (auto scc : set->sccs) {
      this->sccToSetMap.insert(std::make_pair(scc, set));
    }
  }

  /*
   * Create edges between sets according to the custom mapping provided
   * NOTE: This is done instead of using each SCC's edges to handle ignoring certain SCCs
   * within the SCCDAG that are not to be partitioned
   *
   * Only one edge between two sets should be constructed even if multiple SCCs
   * within the two sets have edges between each other
   */
  for (auto sccAndParents : sccToParentsMap) {
    auto scc = sccAndParents.first;
    if (sccToSetMap.find(scc) == sccToSetMap.end()) continue;

    auto selfSet = sccToSetMap.at(scc);
    auto selfNode = this->fetchNode(selfSet);
    auto &parents = sccAndParents.second;

    for (auto parent : parents) {
      if (sccToSetMap.find(parent) == sccToSetMap.end()) continue;
      auto parentSet = sccToSetMap.at(parent);
      auto parentNode = this->fetchNode(parentSet);

      if (this->fetchEdges(parentNode, selfNode).size() != 0) continue;
      this->addEdge(parentSet, selfSet);
    }
  }
}

SCCDAGPartition::~SCCDAGPartition () {

  /*
   * Delete the heap-allocated sets holding onto SCCs
   * NOTE: Do not delete the SCCs themselves; they belong to a different graph
   */
  for (auto node : getNodes()) {
    auto set = node->getT();
    delete set;
  }

}

SCC *SCCDAGPartition::sccOfValue (Value *V) {
  return sccdag->sccOfValue(V);
}

SCCSet *SCCDAGPartition::setOfSCC (SCC *scc) {
  assert(isIncludedInPartitioning(scc) && "SCCDAGPartition: SCC not in any partition");
  return this->sccToSetMap.at(scc);
}

bool SCCDAGPartition::isIncludedInPartitioning (SCC *scc) {
  return this->sccToSetMap.find(scc) != this->sccToSetMap.end();
}

void SCCDAGPartition::mergeSetsAndCollapseResultingCycles (std::unordered_set<SCCSet *> sets) {
  mergeSets(sets);
  collapseCycles();
}

void SCCDAGPartition::mergeSets (std::unordered_set<SCCSet *> sets) {

  /*
   * Merge sets into a single new set
   * Re-map member SCCs to point to this new set
   * Add this set to a new node in the graph
   */
  auto mergedSet = new SCCSet();
  // errs() << "Merging:\n";
  for (auto set : sets) {
    mergedSet->sccs.insert(set->sccs.begin(), set->sccs.end());

    for (auto scc : set->sccs) {
      // scc->printMinimal(errs() << "SCC:\n"); errs() << "\n";
      this->sccToSetMap[scc] = mergedSet;
    }
  }
  auto mergedSetNode = this->addNode(mergedSet, /*inclusion=*/true);

  /*
   * For each set's node,
   * 1) transfer incoming edges to ones on the merged set
   * 2) transfer outgoing edges to ones on the merged set
   * Do so for each edge UNLESS they are between sets now merged into the single set
   *
   * Only one edge between any two subsets should exist
   */
  for (auto set : sets) {
    auto setNode = this->fetchNode(set);

    for (auto edge : setNode->getIncomingEdges()) {
      auto parentSet = edge->getOutgoingT();
      auto parentNode = this->fetchNode(parentSet);

      auto anySCCInParentSet = *parentSet->sccs.begin();
      if (this->sccToSetMap.at(anySCCInParentSet) == mergedSet) continue;

      if (this->fetchEdges(parentNode, mergedSetNode).size() != 0) continue;
      this->addEdge(parentSet, mergedSet);
    }

    for (auto edge : setNode->getOutgoingEdges()) {
      auto childSet = edge->getIncomingT();
      auto childNode = this->fetchNode(childSet);

      auto anySCCInChildSet = *childSet->sccs.begin();
      if (this->sccToSetMap.at(anySCCInChildSet) == mergedSet) continue;

      if (this->fetchEdges(mergedSetNode, childNode).size() != 0) continue;
      this->addEdge(mergedSet, childSet);
    }
  }

  /*
   * Delete old nodes and their now obsolete sets
   */
  for (auto set : sets) {
    auto node = this->fetchNode(set);
    this->removeNode(node);
    delete set;
  }
}

void SCCDAGPartition::collapseCycles (void) {

  /*
   * Use Tarjan's algorithm to collapse all cycles
   */
  std::set<std::unordered_set<SCCSet *> *> collapsedSets;
  std::unordered_set<SCCSet *> visited;
  for (auto nodeToVisit : getNodes()) {
    auto setToVisit = nodeToVisit->getT();
    if (visited.find(setToVisit) != visited.end()) continue;

    /*
     * To use the DGGraphWrapper, the entry node must be set to the node
     * where the traversal starts from
     */
    setEntryNode(nodeToVisit);
    DGGraphWrapper<SCCDAGPartition, SCCSet> wrapper(this);
    // nodeToVisit->print(errs() << "Visiting node using scc_iterator\n");

    /*
     * scc_iterator collects all cycles found from the entry node to leaves of the graph
     * reachable by that node
     */ 
    for (auto dgI = scc_begin(&wrapper); dgI != scc_end(&wrapper); ++dgI) {

      /*
       * Fetch a newly identified cycle of SCC sets.
       */
      const std::vector<DGNodeWrapper<SCCSet> *> &setNodes = *dgI;
      auto firstNodeWrapper = *setNodes.begin();
      auto firstSet = firstNodeWrapper->wrappedNode->getT();
      if (visited.find(firstSet) != visited.end()) {
        continue;
      }

      auto unwrappedSets = new std::unordered_set<SCCSet *>();
      for (auto setWrapper : setNodes) {
        auto unwrappedNode = setWrapper->wrappedNode;
        auto unwrappedSet = unwrappedNode->getT();
        unwrappedSets->insert(unwrappedSet);
        // for (auto scc : unwrappedSet->sccs) {
        //   scc->printMinimal(errs() << "Contained SCC:\n");
        // }
      }

      /*
       * Collapse sets that form a cycle into one set
       */
      visited.insert(unwrappedSets->begin(), unwrappedSets->end());
      collapsedSets.insert(unwrappedSets);
    }
  }

  for (auto setsToMerge : collapsedSets) {
    if (setsToMerge->size() > 1) {
      this->mergeSets(*setsToMerge);
    }
    delete setsToMerge;
  }
}

std::vector<SCCSet *> SCCDAGPartition::getDepthOrderedSets (void) {
  std::vector<SCCSet *> depthOrderedSets;
  std::unordered_set<SCCSet *> encountered;
  auto rootNodes = this->getTopLevelNodes();
  assert(rootNodes.size() != 0
    && "A cycle exists and SCCDAGPartition sets cannot be depth ordered");

  std::queue<SCCSet *> setsToCheck;
  for (auto rootNode : rootNodes) {
    auto root = rootNode->getT();
    setsToCheck.push(root);
  }

  while (!setsToCheck.empty()) {
    auto set = setsToCheck.front();
    auto node = this->fetchNode(set);
    setsToCheck.pop();

    if (encountered.find(set) != encountered.end()) continue;

    /*
     * Confirm all parents have been encountered before counting this node
     */
    if (node->numIncomingEdges() > 0) {
      bool parentsEncountered = true;

      /*
       * If any edge to a parent is to one not encountered yet, do not traverse
       * NOTE: One of the parents not encountered will re-enqueue this node
       */
      for (auto edge : node->getIncomingEdges()) {
        auto parentSet = edge->getOutgoingT();
        parentsEncountered &= encountered.find(parentSet) != encountered.end();
      }
      if (!parentsEncountered) continue;
    }

    /*
     * Add set and enqueue children which have not been added/encountered already
     */
    encountered.insert(set);
    depthOrderedSets.push_back(set);
    for (auto edge : node->getOutgoingEdges()) {
      auto childSet = edge->getIncomingT();
      setsToCheck.push(childSet);
    }
  }

  /*
   * If not all subsets were encountered, at some point a cycle was encountered
   *  that prevented a subset-depth traversal from completing
   */
  assert(depthOrderedSets.size() == this->numNodes()
    && "A cycle exists and SCCDAGPartition sets cannot be depth ordered");
  return depthOrderedSets;
}

SCCDAG *SCCDAGPartition::getSCCDAG (void) const {
  return this->sccdag;
}

SCCDAGPartitioner::SCCDAGPartitioner (
  SCCDAG *sccdag,
  std::unordered_set<SCCSet *> initialSets,
  std::unordered_map<SCC *, std::unordered_set<SCC *>> sccToParentsMap,
  StayConnectedNestedLoopForestNode *loopNode
) : rootLoop{loopNode} {

  this->partition = new SCCDAGPartition(sccdag, initialSets, sccToParentsMap);
  for (auto l : loopNode->getNodes()){
    this->allLoops.insert(l);
  }

  resetPartitioner();
}

SCCDAGPartitioner::~SCCDAGPartitioner () {
  delete this->partition;
}

uint64_t SCCDAGPartitioner::numberOfPartitions (void){
  return partition->numNodes();
}

void SCCDAGPartitioner::resetPartitioner (void) {

  /*
   * Create a program forward ordering of SCCs for debug purposes.
   * This SCC order will be stored in SCCDebugOrder.
   *
   *  - Reset SCCDebugOrder.
   */
  auto numberOfSCCs = 0;
  for (auto node : partition->getNodes()) {
    numberOfSCCs += node->getT()->sccs.size();
  }
  SCCDebugOrder.resize(numberOfSCCs);
  auto count = 0;

  /*
   *  - Fetch the header basic block of the outermost loop. This will be the beginning of the traversal.
   */
  auto const topLoop = this->rootLoop->getLoop();
  auto bb = topLoop->getHeader();

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
    if (!topLoop->isIncluded(bb)){

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
      auto scc = partition->sccOfValue(&I);
      if (!partition->isIncludedInPartitioning(scc)) continue;

      /*
       * Check whether the current SCC has been processed already.
       */
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
  if (SCCDebugIndex.size() != numberOfSCCs) {
    errs() << "ERROR: Mismatch # of SCC encountered (in program forward order traversal): "
      << SCCDebugIndex.size() << " versus total # of SCC in subsets: "
      << numberOfSCCs << "\n";
    assert(false && "SCCDAGPartition::resetPartitioner");
  }

  return ;
}

bool SCCDAGPartitioner::isAncestor (SCCSet *parentTarget, SCCSet *target) {
  std::queue<SCCSet *> setToCheck;
  setToCheck.push(target);

  while (!setToCheck.empty()) {
    auto set = setToCheck.front();
    setToCheck.pop();

    auto node = partition->fetchNode(set);
    if (node->numIncomingEdges() == 0) continue;

    for (auto edge : node->getIncomingEdges()) {
      auto parentSet = edge->getOutgoingT();
      if (parentSet == parentTarget) return true;
      setToCheck.push(parentSet);
    }
  }

  return false;
}

std::pair<SCCSet *, SCCSet *> SCCDAGPartitioner::getParentChildPair (SCCSet *setA, SCCSet *setB) {
  SCCSet * parent;
  SCCSet * child;
  if (isAncestor(setA, setB)) {
    return std::make_pair(setA, setB);
  } else if (isAncestor(setB, setA)) {
    return std::make_pair(setB, setA);
  } else return std::make_pair(nullptr, nullptr);
}

std::unordered_set<SCCSet *> SCCDAGPartitioner::getOverlap(std::unordered_set<SCCSet *> setsA, std::unordered_set<SCCSet *> setsB) {
  std::unordered_set<SCCSet *> overlap;
  for (auto set : setsA) {
    if (setsB.find(set) == setsB.end()) continue;
    overlap.insert(set);
  }
  return overlap;
}

bool SCCDAGPartitioner::isMergeIntroducingCycle (SCCSet *setA, SCCSet *setB) {
  auto parentChild = getParentChildPair(setA, setB);
  if (parentChild.first == nullptr) return false;

  /*
   * If one set is the ancestor of another, no cycle is created ONLY if
   * no set can be reached by the parent that can reach the child
   */
  auto parentDescendants = getDescendants(parentChild.first);
  auto childAncestors = getAncestors(parentChild.second);
  auto overlap = getOverlap(parentDescendants, childAncestors);
  return overlap.size() > 0;
}

std::unordered_set<SCCSet *> SCCDAGPartitioner::getCycleIntroducedByMerging (SCCSet *setA, SCCSet *setB) {
  auto parentChild = getParentChildPair(setA, setB);
  if (parentChild.first == nullptr) {
    return { setA, setB };
  }

  auto parentDescendants = getDescendants(parentChild.first);
  auto childAncestors = getAncestors(parentChild.second);
  auto overlap = getOverlap(parentDescendants, childAncestors);
  overlap.insert(parentChild.first);
  overlap.insert(parentChild.second);
  return overlap;
}

std::unordered_set<SCCSet *> SCCDAGPartitioner::getDescendants (SCCSet *startingSet) {
  std::unordered_set<SCCSet *> descendants;
  std::queue<SCCSet *> setToCheck;
  setToCheck.push(startingSet);

  while (!setToCheck.empty()) {
    auto set = setToCheck.front();
    setToCheck.pop();

    auto node = partition->fetchNode(set);
    if (node->numOutgoingEdges() == 0) continue;

    for (auto edge : node->getOutgoingEdges()) {
      auto childSet = edge->getIncomingT();
      if (descendants.find(childSet) != descendants.end()) continue;
      setToCheck.push(childSet);
      descendants.insert(childSet);
    }
  }

  return descendants;
}

std::unordered_set<SCCSet *> SCCDAGPartitioner::getAncestors (SCCSet *startingSet) {
  std::unordered_set<SCCSet *> ancestors;
  std::queue<SCCSet *> setToCheck;
  setToCheck.push(startingSet);

  while (!setToCheck.empty()) {
    auto set = setToCheck.front();
    setToCheck.pop();

    auto node = partition->fetchNode(set);
    if (node->numIncomingEdges() == 0) continue;

    for (auto edge : node->getIncomingEdges()) {
      auto parentSet = edge->getOutgoingT();
      if (ancestors.find(parentSet) != ancestors.end()) continue;
      setToCheck.push(parentSet);
      ancestors.insert(parentSet);
    }
  }

  return ancestors;
}

SCCDAGPartition *SCCDAGPartitioner::getPartitionGraph (void) {
  return this->partition;
}

std::unordered_set<SCCSet *> SCCDAGPartitioner::getParents (SCCSet *set) {
  std::unordered_set<SCCSet *> parents;
  auto node = partition->fetchNode(set);
  for (auto edge : node->getIncomingEdges())  {
    auto parentSet = edge->getOutgoingT();
    parents.insert(parentSet);
  }

  return parents;
}

std::unordered_set<SCCSet *> SCCDAGPartitioner::getChildren (SCCSet *set) {
  std::unordered_set<SCCSet *> children;
  auto node = partition->fetchNode(set);
  for (auto edge : node->getOutgoingEdges())  {
    auto childSet = edge->getIncomingT();
    children.insert(childSet);
  }

  return children;
}

std::unordered_set<SCCSet *> SCCDAGPartitioner::getSets (void) {
  std::unordered_set<SCCSet *> sets;
  for (auto node : partition->getNodes()) {
    sets.insert(node->getT());
  }
  return sets;
}

std::unordered_set<SCCSet *> SCCDAGPartitioner::getRoots (void) {
  std::unordered_set<SCCSet *> sets;
  for (auto node : partition->getTopLevelNodes()) {
    sets.insert(node->getT());
  }
  return sets;
}

std::vector<SCCSet *> SCCDAGPartitioner::getDepthOrderedSets (void) {
  return partition->getDepthOrderedSets();
}

SCCSet *SCCDAGPartitioner::mergePair (SCCSet *setA, SCCSet *setB) {
  this->partition->mergeSetsAndCollapseResultingCycles({ setA, setB });
  auto anySCCInMergedSet = *setA->sccs.begin();
  auto mergedSet = this->partition->setOfSCC(anySCCInMergedSet);
  return mergedSet;
}

void SCCDAGPartitioner::mergeAllPairs (std::set<std::pair<SCC *, SCC *>> pairs) {
  for (auto pair : pairs) {
    auto setProducer = this->partition->setOfSCC(pair.first);
    auto setConsumer = this->partition->setOfSCC(pair.second);
    if (setProducer == setConsumer) continue;

    this->partition->mergeSetsAndCollapseResultingCycles({ setProducer, setConsumer });
  }
}
/*
void SCCDAGPartitioner::mergeLoopCarriedDependencies (LoopCarriedDependencies *LCD) {

  /*
   * Collect all pairs of SCC that need to be merged into the same set 
   */
/*  std::set<std::pair<SCC *, SCC *>> lcdPairs{};
  for (auto loop : allLoops) {
    auto loopCarriedEdges = LCD->getLoopCarriedDependenciesForLoop(*loop);
    for (auto edge : loopCarriedEdges) {
      if (!edge->isMemoryDependence()) continue;

      auto producer = edge->getOutgoingT();
      auto consumer = edge->getIncomingT();

      /*
       * NOTE: All SCC with a loop carried dependency must be partition-able
       * if this API is being called to merge them together
       */
/*      auto producerSCC = this->partition->sccOfValue(producer);
      auto consumerSCC = this->partition->sccOfValue(consumer);
      if (!this->partition->isIncludedInPartitioning(producerSCC)) continue;
      if (!this->partition->isIncludedInPartitioning(consumerSCC)) continue;
      if (producerSCC == consumerSCC) continue;

      lcdPairs.insert(std::make_pair(producerSCC, consumerSCC));
    }
  }

  mergeAllPairs(lcdPairs);
}
*/
void SCCDAGPartitioner::mergeLCSSAPhisWithTheValuesTheyPropagate (void) {

  /*
   * Collect all LCSSA PHIs contained in the root loop
   * that propagate nested loop values
   */
  std::unordered_set<PHINode *> lcssaPHIs;
  for (auto loopNode : this->allLoops) {
    if (this->rootLoop == loopNode) {
      continue;
    }
    auto loop = loopNode->getLoop();
    for (auto exitBlock : loop->getLoopExitBasicBlocks()) {
      for (auto &phi : exitBlock->phis()) {
        lcssaPHIs.insert(&phi);
      }
    }
  }

  /*
   * Merge these PHIs with the sets containing their incoming values
   * if those incoming values belong to another set
   */
  std::set<std::pair<SCC *, SCC *>> lcssaPairs{};
  for (auto phi : lcssaPHIs) {
    auto consumerSCC = this->partition->sccOfValue(phi);
    if (!this->partition->isIncludedInPartitioning(consumerSCC)) continue;

    for (auto i = 0; i < phi->getNumIncomingValues(); ++i) {
      auto incomingValue = phi->getIncomingValue(i);
      auto producerSCC = this->partition->sccOfValue(incomingValue);
      if (!producerSCC) continue;
      if (!this->partition->isIncludedInPartitioning(producerSCC)) continue;

      lcssaPairs.insert(std::make_pair(producerSCC, consumerSCC));
    }
  }

  mergeAllPairs(lcssaPairs);
}

raw_ostream &SCCSet::print (raw_ostream &stream) {
  stream << "SCC set:\n";
  for (auto scc : sccs) {
    auto anyNode = *scc->begin_nodes();
    anyNode->getT()->print(stream << "\tSCC containing: ");
    stream << "\n";
  }
  return stream;
}

void SCCDAGPartitioner::mergeAlongMemoryEdges (void) {
  std::set<std::pair<SCC *, SCC *>> memoryPairs{};
  for (auto edge : this->partition->getSCCDAG()->getEdges()) {

    bool containsMemoryDependence = false;
    for (auto subEdge : edge->getSubEdges()) {
      if (!subEdge->isMemoryDependence()) continue;
      containsMemoryDependence = true;
      break;
    }
    if (!containsMemoryDependence) continue;

    auto producerSCC = edge->getOutgoingT();
    auto consumerSCC = edge->getIncomingT();
    if (!this->partition->isIncludedInPartitioning(producerSCC)) continue;
    if (!this->partition->isIncludedInPartitioning(consumerSCC)) continue;

    memoryPairs.insert(std::make_pair(producerSCC, consumerSCC));
  }

  mergeAllPairs(memoryPairs);
}

raw_ostream &SCCDAGPartitioner::printSet (raw_ostream &stream, SCCSet *set) {
  stream << "Set: ";
  for (auto scc : set->sccs) {
    stream << this->SCCDebugIndex[scc] << " ";
  }
  return stream << "\n";
}

// raw_ostream &SCCDAGPartition::print (raw_ostream &stream, std::string prefix) {
//   printSCCIndices(stream, prefix);
//   printGraph(stream, prefix);
//   return stream;
// }

// raw_ostream &SCCDAGPartition::printSCCIndices (raw_ostream &stream, std::string prefix) {
//   for (auto i = 0; i < SCCDebugOrder.size(); ++i) {
//     stream << prefix << "SCC: " << i << " Internal nodes:\n";
//     for (auto nodePair : SCCDebugOrder[i]->internalNodePairs()) {
//       nodePair.first->print(stream << prefix << "  ");
//       stream << "\n";
//     }
//   }
//   return stream;
// }

// std::string SCCDAGPartition::subsetStr (SCCset *subset) {
//   std::string text = "Set: ";
//   raw_string_ostream str(text);
//   for (auto scc : *subset) str << " " << SCCDebugIndex[scc];
//   str.str();
//   return text;
// };

// raw_ostream &SCCDAGPartition::printNodeInGraph (raw_ostream &stream, std::string prefix, SCCset *subset) {
//   stream << prefix << subsetStr(subset) << "\n" << prefix << " Children: ";
//   if (childrenSubsets.find(subset) != childrenSubsets.end()) {
//     for (auto child : childrenSubsets[subset]) stream << subsetStr(child) << "; ";
//   }
//   stream << "\n" << prefix << " Parents: ";
//   if (parentSubsets.find(subset) != parentSubsets.end()) {
//     for (auto parent : parentSubsets[subset]) stream << subsetStr(parent) << "; ";
//   }
//   stream << "\n";
//   return stream;
// }

// raw_ostream &SCCDAGPartition::printGraph (raw_ostream &stream, std::string prefix) {
//   std::set<SCCset *> subsetsEncountered;
//   for (auto scc : SCCDebugOrder) {
//     auto subset = SCCToSet[scc];
//     if (subsetsEncountered.find(subset) != subsetsEncountered.end()) continue;
//     subsetsEncountered.insert(subset);
//     printNodeInGraph(stream, prefix, subset);
//   }
//   return stream;
// }

