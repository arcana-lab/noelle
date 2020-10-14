/*
 * Copyright 2019 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SCCPartitionScheduler.hpp"

using namespace llvm;
using namespace llvm::noelle;

SCCPartitionScheduler::SCCPartitionScheduler(
  SCCDAG *loopSCCDAG,
  std::unordered_set<SCCSet *> sccPartitions,
  DataFlowResult *reachabilityDFR
) : loopSCCDAG{loopSCCDAG}, sccPartitions{sccPartitions}, reachabilityDFR{reachabilityDFR} {
}

bool SCCPartitionScheduler::squeezePartitions (void) {
  auto modified = false;

  /*
   * Collect a mapping between a SCC and its partition
   */
  for (auto partition : sccPartitions) {
    for (auto scc : partition->sccs) {
      sccToPartitionMap[scc] = partition;
    }
  }

  /*
   * Collect reverse reachability OUT sets
   * NOTE: This is done on EVERY SCC, not just ones that are partitioned
   * This is to ensure a full picture of reachability
   *
   * Collect a mapping between basic blocks and partitions present in that basic block
   */
  for (auto sccNode : loopSCCDAG->getNodes()) {
    auto scc = sccNode->getT();
    scc->iterateOverInstructions([&](Instruction *I) -> bool {

      if (sccToPartitionMap.find(scc) != sccToPartitionMap.end()) {
        auto B = I->getParent();
        auto partition = sccToPartitionMap[scc];
        basicBlockToPartitionsMap[B].insert(partition);
      }

      auto instructionsAfterI = reachabilityDFR->OUT(I);
      for (auto J : instructionsAfterI) {
        reverseReachabilityMap[cast<Instruction>(J)].insert(I);
      }

      return false;
    });
  }

  /*
   * For every basic block, determine whether a partition belongs
   * at the top or the bottom of the block
   */
  for (auto &[B, partitions] : basicBlockToPartitionsMap) {

    /*
     * For each partition, pull each instruction towards its preferred direction
     * as much as dependencies would allow
     */
    for (auto partitionToSqueeze : partitions) {

      /*
       * Check if the partition extends before this basic block or after
       * If it only extends before, then squeeze and schedule the SCC higher
       * Else, squeeze and schedule the SCC lower
       * TODO:
       */
      bool isHoistingPartition = true;

      // for (auto scc : partitionToSqueeze->sccs) {
      //   for (auto [value, node] : scc->internalNodePairs()) {
      //     value->print(errs() << "Adjusting: "); errs() << "\n";
      //   }
      // }
      // B->print(errs() << "Adjusting within\n"); errs() << "\n";

      /*
       * Collect instructions in the basic block belonging to the partition
       *
       * Also collect any consumed dependencies on the partition instructions
       * that may need to be hoisted in order to hoist partition instructions
       */
      std::unordered_set<Instruction *> dependedOnInstructions{};
      std::unordered_set<Instruction *> partitionInstructions{};
      for (auto &I : *B) {

        /*
         * Ignore instructions not part of the partition
         * Skip PHIs and the terminator
         */
        if (isa<PHINode>(&I) || I.isTerminator()) continue;
        auto partitionOfI = getPartition(&I);
        if (partitionOfI != partitionToSqueeze) continue;

        partitionInstructions.insert(&I);
        for (auto dependedOnI : collectDependedOnInstructionsWithinBlock(&I)) {
          if (isa<PHINode>(dependedOnI) || dependedOnI->isTerminator()) continue;
          dependedOnInstructions.insert(dependedOnI);
        }
      }
      if (partitionInstructions.size() < 1) continue;

      /*
       * Order the instructions so they cdan be hoisted in one sweep
       * Stop at the last partition instruction so as not to hoist depended on
       * instructions that can already follow the partition (i.e. memory dependencies)
       * NOTE: Keep in mind that @dependedOnInstructions can hold instructions
       * in @partitionInstructions, so check @partitionInstructions first
       */
      std::vector<Instruction *> orderedInstructionsToHoist{};
      for (auto &I : *B) {

        if (partitionInstructions.find(&I) != partitionInstructions.end()) {
          partitionInstructions.erase(&I);
          orderedInstructionsToHoist.push_back(&I);
        } else if (dependedOnInstructions.find(&I) != dependedOnInstructions.end()) {
          orderedInstructionsToHoist.push_back(&I);
        } else continue;

        if (partitionInstructions.size() == 0) break;
      }

      auto targetInstruction = *orderedInstructionsToHoist.begin();
      for (auto I : orderedInstructionsToHoist) {

        /*
         * Attempt to pull the instruction towards the previous target
         * Reset the target to directly after the hoisted instruction
         */
        auto initialNextNode = I->getNextNode();
        // I->print(errs() << "Moving: "); errs() << "\n";
        // if (targetInstruction) { targetInstruction->print(errs() << "Going to move to: "); errs() << "\n"; }
        targetInstruction = hoistInstructionTowards(I, targetInstruction);
        auto finalNextNode = I->getNextNode();
        modified |= initialNextNode != finalNextNode;

        // if (initialNextNode != finalNextNode) {
        //   I->print(errs() << "Moved: "); errs() << "\n";
        // }
      }
    }
  }

  return modified;
}

SCCSet *SCCPartitionScheduler::getPartition (Instruction *I) {
  auto scc = loopSCCDAG->sccOfValue(I);
  if (sccToPartitionMap.find(scc) == sccToPartitionMap.end()) return nullptr;
  return sccToPartitionMap.at(scc);
}

Instruction *SCCPartitionScheduler::hoistInstructionTowards (Instruction *instructionToMove, Instruction *target) {
  if (target == instructionToMove) return instructionToMove;

  /*
   * Identify all consumed dependencies of the instruction to move
   */
  std::unordered_set<Instruction *> consumedInstructions = collectDependedOnInstructionsWithinBlock(instructionToMove);

  /*
   * Search the basic block from the instruction to move up
   * until the target or a consumed dependency is reached
   */
  auto instructionToHoistTo = instructionToMove;
  while (instructionToHoistTo && (target != instructionToHoistTo)) {
    if (consumedInstructions.find(instructionToHoistTo) != consumedInstructions.end()) break;
    instructionToHoistTo = instructionToHoistTo->getPrevNode();
  }

  /*
   * If the instruction to hoist to is the nullptr, we reached the beginning of the basic block
   * and can hoist all the way to the beginning of the block
   */
  auto block = instructionToMove->getParent();
  if (!instructionToHoistTo) {
    auto firstInst = &*block->begin();
    if (firstInst != instructionToMove) {
      instructionToMove->moveBefore(firstInst);
    }
    return instructionToMove;
  }

  /*
   * Some instruction to hoist to was found, limited by dependencies or the target,
   * so we hoist to right after that instruction
   */
  instructionToMove->moveAfter(instructionToHoistTo);
  return instructionToMove;
}

Instruction *SCCPartitionScheduler::sinkInstructionTowards (Instruction *instructionToMove, Instruction *target) {
  return nullptr;
}

std::unordered_set<Instruction *> SCCPartitionScheduler::collectDependedOnInstructionsWithinBlock (Instruction *I) {

  std::unordered_set<Instruction *> consumedInstructions;
  auto blockOfI = I->getParent();
  auto sccOfI = loopSCCDAG->sccOfValue(I);
  auto nodeOfI = sccOfI->fetchNode(I);

  std::queue<DGNode<Value> *> nodes;
  nodes.push(nodeOfI);
  while (!nodes.empty()) {
    auto node = nodes.front();
    nodes.pop();

    for (auto edge : node->getIncomingEdges()) {

      /*
       * Ignore self edges
       */
      auto consumedNode = edge->getOutgoingNode();
      auto consumedValue = edge->getOutgoingT();
      if (consumedValue == I) continue;

      /*
       * Only catalog consumed instructions within the consumer's basic block
       * Ignore already consumed instructions
       */
      if (auto consumedInst = dyn_cast<Instruction>(consumedValue)) {
        auto consumedBlock = consumedInst->getParent();
        if (consumedBlock != blockOfI) continue;

        if (consumedInstructions.find(consumedInst) != consumedInstructions.end()) continue;
        consumedInstructions.insert(consumedInst);

        nodes.push(consumedNode);
      }
    }
  }

  return consumedInstructions;
}