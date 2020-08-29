/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"
#include "HELIXTask.hpp"
#include <set>

using namespace llvm ;

void HELIX::spillLoopCarriedDataDependencies (LoopDependenceInfo *LDI) {

  /*
   * Fetch the task
   */
  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);

  /*
   * Fetch the header.
   */
  auto loopSummary = LDI->getLoopStructure();
  auto loopHeader = loopSummary->getHeader();
  auto loopPreHeader = loopSummary->getPreHeader();
  auto clonedPreheader = helixTask->getCloneOfOriginalBasicBlock(loopPreHeader);

  /*
   * Fetch the loop function.
   */
  auto loopFunction = loopSummary->getFunction();
  auto sccdag = LDI->sccdagAttrs.getSCCDAG();

  /*
   * Collect all PHIs in the loop header; they are local variables
   * with loop carried data dependencies and need to be spilled
   * NOTE: There need not be a single loop carried PHI that needs spilling.
   * Non-independent function calls and already-in-memory data are such examples.
   */
  std::vector<PHINode *> originalLoopCarriedPHIs;
  std::vector<PHINode *> clonedLoopCarriedPHIs;
  for (auto &phi : loopHeader->phis()) {
    auto phiSCC = sccdag->sccOfValue(cast<Value>(&phi));
    auto sccInfo = LDI->sccdagAttrs.getSCCAttrs(phiSCC);

    if (sccInfo->canExecuteReducibly()) continue;
    if (sccInfo->isInductionVariableSCC()) continue;

    originalLoopCarriedPHIs.push_back(&phi);
    auto clonePHI = (PHINode *)(helixTask->getCloneOfOriginalInstruction(&phi));
    clonedLoopCarriedPHIs.push_back(clonePHI);
  }

  /*
   * Register each PHI as part of the loop carried environment
   */
  std::vector<Type *> phiTypes;
  std::set<int> nonReducablePHIs;
  std::set<int> cannotReduceLoopCarriedPHIs;
  for (auto i = 0; i < clonedLoopCarriedPHIs.size(); ++i) {
    auto phiType = clonedLoopCarriedPHIs[i]->getType();
    phiTypes.push_back(phiType);
    nonReducablePHIs.insert(i);
  }

  /*
   * Instantiate a builder to the task's entry, track the terminator,
   * and later hoist the terminator back to the end of the entry block.
   */
  auto entryBlock = helixTask->getEntry();
  auto entryBlockTerminator = entryBlock->getTerminator();
  IRBuilder<> entryBuilder(entryBlockTerminator);

  /*
   * Register a new environment builder and the single HELIX task
   */
  this->loopCarriedEnvBuilder = new EnvBuilder(module.getContext());
  this->loopCarriedEnvBuilder->createEnvVariables(phiTypes, nonReducablePHIs, cannotReduceLoopCarriedPHIs, 1);
  this->loopCarriedEnvBuilder->createEnvUsers(1);

  /*
   * Fetch the unique user of the environment builder dedicated to spilled variables.
   */
  auto envUser = this->loopCarriedEnvBuilder->getUser(0);

  envUser->setEnvArray(entryBuilder.CreateBitCast(
    helixTask->loopCarriedArrayArg,
    PointerType::getUnqual(loopCarriedEnvBuilder->getEnvArrayTy())
  ));

  /*
   * Allocate the environment array (64 byte aligned)
   * Load incoming values from the preheader
   */
  IRBuilder<> loopFunctionBuilder(&*loopFunction->begin()->begin());
  loopCarriedEnvBuilder->generateEnvArray(loopFunctionBuilder);
  loopCarriedEnvBuilder->generateEnvVariables(loopFunctionBuilder);

  IRBuilder<> builder(this->entryPointOfParallelizedLoop);
  for (auto envIndex = 0; envIndex < originalLoopCarriedPHIs.size(); ++envIndex) {
    auto phi = originalLoopCarriedPHIs[envIndex];
    auto preHeaderIndex = phi->getBasicBlockIndex(loopPreHeader);
    auto preHeaderV = phi->getIncomingValue(preHeaderIndex);
    builder.CreateStore(preHeaderV, loopCarriedEnvBuilder->getEnvVar(envIndex));
  }

  std::unordered_map<BasicBlock *, BasicBlock *> cloneToOriginalBlockMap;
  for (auto originalB : helixTask->getOriginalBasicBlocks()) {
    auto cloneB = helixTask->getCloneOfOriginalBasicBlock(originalB);
    cloneToOriginalBlockMap.insert(std::make_pair(cloneB, originalB));
  }

  /*
   * Generate code to store each incoming loop carried PHI value,
   * load the incoming value, and replace PHI uses with load uses
   * For the pre header edge case, store this initial value at time of
   * allocation of the environment
   */
  for (auto phiI = 0; phiI < clonedLoopCarriedPHIs.size(); phiI++) {
    auto originalPHI = originalLoopCarriedPHIs[phiI];
    auto clonePHI = clonedLoopCarriedPHIs[phiI];
    auto spilled = new SpilledLoopCarriedDependency();
    this->spills.insert(spilled);
    spilled->originalLoopCarriedPHI = originalPHI;
    spilled->loopCarriedPHI = clonePHI;

    /*
     * Track the initial value of this spilled variable
     */
    spilled->clonedInitialValue = clonePHI->getIncomingValueForBlock(clonedPreheader);

    /*
     * Create GEP access of the environment variable at index i
     */
    envUser->createEnvPtr(entryBuilder, phiI, phiTypes[phiI]);
    auto envPtr = envUser->getEnvPtr(phiI);

    createLoadsAndStoresToSpilledLCD(LDI, cloneToOriginalBlockMap, spilled, envPtr);
  }

  return ;
}

void HELIX::createLoadsAndStoresToSpilledLCD (
  LoopDependenceInfo *LDI,
  std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
  SpilledLoopCarriedDependency *spill,
  Value *spillEnvPtr
) {

  /*
   * Fetch task and loop
   */
  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);
  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto originalLoopFunction = loopHeader->getParent();
  DominatorTree originalLoopDT(*originalLoopFunction);
  PostDominatorTree originalLoopPDT(*originalLoopFunction);
  DominatorSummary DS(originalLoopDT, originalLoopPDT);

  /*
   * Store loop carried dependencies into the spill environment
   * Identify the basic block dominating all stores to the spill environment
   */
  auto originalStoreDominatingBlock = insertStoresToSpilledLCD(
    LDI,
    cloneToOriginalBlockMap,
    spill,
    spillEnvPtr,
    &DS
  );

  /*
   * HACK: We cannot handle communicating to ParallelizationTechnique that an original
   * value is cloned in multiple places, so if there are more than one exit, we have to
   * default to producing the spill load in the header (which is NOT efficient at all when synchronizing)
   */
  if (loopStructure->getLoopExitBasicBlocks().size() > 1) {
    auto clonedHeader = helixTask->getCloneOfOriginalBasicBlock(loopHeader);
    IRBuilder<> headerBuilder(clonedHeader->getFirstNonPHIOrDbgOrLifetime());
    auto headerLoad = headerBuilder.CreateLoad(spillEnvPtr);
    helixTask->addInstruction(spill->originalLoopCarriedPHI, headerLoad);
    spill->loopCarriedPHI->replaceAllUsesWith(headerLoad);
    spill->loopCarriedPHI->eraseFromParent();
    return;
  }

  /*
   * Define a frontier across the loop extending out from users of the spill
   * This frontier will determine where to insert any needed loads
   * so that the value of the spill environment is known every iteration and
   * can be propagated to the header for potential use in the live out environment
   */
  std::unordered_set<BasicBlock *> originalFrontierBlocks;
  defineFrontierForLoadsToSpilledLCD(
    LDI,
    cloneToOriginalBlockMap,
    spill,
    &DS,
    originalFrontierBlocks,
    originalStoreDominatingBlock
  );

  for (auto frontierBlock : originalFrontierBlocks) {
    frontierBlock->print(errs() << "Frontier block:\n"); errs() << "\n";
  }

  replaceUsesOfSpilledPHIWithLoads(
    LDI,
    cloneToOriginalBlockMap,
    spill,
    spillEnvPtr,
    &DS,
    originalFrontierBlocks
  );

  auto exitBlockToValueMap = propagateLoadsOfSpilledLCDToLoopExits(
    LDI,
    cloneToOriginalBlockMap,
    spill,
    spillEnvPtr
  );

  /*
   * Due to the limitation mentioned above, this approach is only used when there is one exit
   */
  auto singleExit = *loopStructure->getLoopExitBasicBlocks().begin();
  auto clonedSingleExit = helixTask->getCloneOfOriginalBasicBlock(singleExit);
  auto exitValue = exitBlockToValueMap.at(clonedSingleExit);
  helixTask->addInstruction(spill->originalLoopCarriedPHI, exitValue);
  spill->loopCarriedPHI->eraseFromParent();

  for (auto load : spill->environmentLoads) {
    load->print(errs() << "Load: "); errs() << "\n";
  }

  return ;
}

BasicBlock *HELIX::insertStoresToSpilledLCD (
  LoopDependenceInfo *LDI,
  std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
  SpilledLoopCarriedDependency *spill,
  Value *spillEnvPtr,
  DominatorSummary *originalLoopDS
) {

  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);
  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto loopPreHeader = loopStructure->getPreHeader();
  auto headerClone = helixTask->getCloneOfOriginalBasicBlock(loopHeader);
  auto preHeaderClone = helixTask->getCloneOfOriginalBasicBlock(loopPreHeader);

  /*
   * Store loop carried values of the PHI into the environment
   */
  for (auto inInd = 0; inInd < spill->loopCarriedPHI->getNumIncomingValues(); ++inInd) {
    auto incomingBB = spill->loopCarriedPHI->getIncomingBlock(inInd);
    if (incomingBB == preHeaderClone) continue;

    /*
     * Determine the position of the incoming value's producer
     * If it s an instruction computed within the loop, insert the store at that point
     * Otherwise, insert at the incoming block's entry
     */
    auto incomingV = spill->loopCarriedPHI->getIncomingValue(inInd);
    Instruction *insertPoint = incomingBB->getTerminator();
    if (auto incomingI = dyn_cast<Instruction>(incomingV)) {
      auto blockOfIncomingI = incomingI->getParent();
      auto originalBlockOfIncomingI = cloneToOriginalBlockMap.at(blockOfIncomingI);
      if (loopStructure->isIncluded(originalBlockOfIncomingI)) {
        insertPoint = isa<PHINode>(incomingI)
          ? blockOfIncomingI->getFirstNonPHIOrDbgOrLifetime() : incomingI->getNextNode();
      }
    }

    IRBuilder<> builder(insertPoint);
    spill->environmentStores.insert(builder.CreateStore(incomingV, spillEnvPtr));
  }

  /*
   * Collect blocks dominating all stores and that are present at the root loop level.
   * These are conservatively safe points to insert loads. Loads cannot be placed in sub-loops
   * as the load has to always reflect the value of the spilled variable from the last iteration
   */
  BasicBlock * originalStoreDominatingBlock = nullptr;
  for (auto store : spill->environmentStores) {
    auto cloneStoreBlock = store->getParent();
    auto originalStoreBlock = cloneToOriginalBlockMap.at(cloneStoreBlock);

    /*
     * The block is at the root loop level. Consider the block for nearest common domination search
     */
    auto nestedMostLoop = LDI->getNestedMostLoopStructure(originalStoreBlock->getTerminator());
    if (nestedMostLoop == loopStructure) {
      originalStoreDominatingBlock = !originalStoreDominatingBlock ? originalStoreBlock
        : originalLoopDS->DT.findNearestCommonDominator(originalStoreDominatingBlock, originalStoreBlock);
      continue;
    }

    /*
     * Find the preheader to the direct child loop of the root loop level containing this store
     */
    auto childLoop = nestedMostLoop;
    while (childLoop->getParentLoop() != loopStructure) {
      childLoop = childLoop->getParentLoop();
    }
    auto childLoopPreheader = childLoop->getPreHeader();

    originalStoreDominatingBlock = !originalStoreDominatingBlock ? childLoopPreheader
      : originalLoopDS->DT.findNearestCommonDominator(originalStoreDominatingBlock, childLoopPreheader);
  }

  return originalStoreDominatingBlock;
}

void HELIX::defineFrontierForLoadsToSpilledLCD (
  LoopDependenceInfo *LDI,
  std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
  SpilledLoopCarriedDependency *spill,
  DominatorSummary *originalLoopDS,
  std::unordered_set<BasicBlock *> &originalFrontierBlocks,
  BasicBlock *originalStoreDominatingBlock
) {

  auto &loopHierarchy = LDI->getLoopHierarchyStructures();
  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();

  /*
   * Collect a partial frontier as close to users of the spill,
   * blocks that dominate the stores to the spill environment
   */
  for (auto user : spill->loopCarriedPHI->users()) {
    auto userInst = cast<Instruction>(user);
    auto cloneUserBlock = userInst->getParent();
    auto originalUserBlock = cloneToOriginalBlockMap.at(cloneUserBlock);

    /*
     * If the user is a PHI, since a load cannot be placed before a PHI,
     * identify a strictly dominating block of the user
     */
    if (auto userPHI = dyn_cast<PHINode>(user)) {
      for (auto i = 0; i < userPHI->getNumIncomingValues(); ++i) {
        auto cloneIncomingBlock = userPHI->getIncomingBlock(i);
        auto originalIncomingBlock = cloneToOriginalBlockMap.at(cloneIncomingBlock);
        originalUserBlock = originalLoopDS->DT.findNearestCommonDominator(originalUserBlock, originalIncomingBlock);
      }
    }

    /*
     * Ensure the user block to be part of the frontier is not in a nested loop
     */
    auto userLoop = loopHierarchy.getLoop(*originalUserBlock);
    if (userLoop != loopStructure) {
      while (userLoop->getParentLoop() != loopStructure) {
        userLoop = userLoop->getParentLoop();
      }
      originalUserBlock = userLoop->getPreHeader();
    }

    /*
     * Find nearest common dominator of the blocks dominating users and stores
     */
    auto originalCommonDominatorBlock =
      originalLoopDS->DT.findNearestCommonDominator(originalStoreDominatingBlock, originalUserBlock);
    originalFrontierBlocks.insert(originalCommonDominatorBlock);
  }

  /*
   * Heuristic for extending out the frontier:
   * - Traverse basic blocks (via BFS) from the loop entry tracking the minimum
   *   depth it takes to reach each block (exclude nested loop blocks)
   * - Take the minimum depth of all basic blocks in the partial frontier (call this depth D1)
   * - Identify the minimum depth of any latch (call this depth D2)
   * - Identify the minimum depth of any loop block that can exit (call this depth D3)
   * - Add all basic blocks at a depth of min(D1, D2, D3)
   */
  std::unordered_map<BasicBlock *, int> blockToMinimumDepthMap;
  std::queue<BasicBlock *> blockQueue;
  blockToMinimumDepthMap.insert({loopHeader, 0});
  blockQueue.push(loopHeader);
  while (!blockQueue.empty()) {
    auto block = blockQueue.front();
    blockQueue.pop();

    assert(blockToMinimumDepthMap.find(block) != blockToMinimumDepthMap.end());
    auto depth = blockToMinimumDepthMap.at(block);


    std::unordered_set<BasicBlock *> successorsToTraverse;
    for (auto successor : successors(block)) {

      /*
       * Skip blocks not in the loop
       * Skip to nested loop exits so the frontier is exclusive to root loop blocks
       */
      if (!loopStructure->isIncluded(successor)) continue;
      auto loopContainingSuccessor = loopHierarchy.getLoop(*successor);
      if (loopContainingSuccessor == loopStructure) {
        successorsToTraverse.insert(successor);
        continue;
      }

      for (auto exit : loopContainingSuccessor->getLoopExitBasicBlocks()) {
        successorsToTraverse.insert(exit);
      }
    }

    /*
     * For all successors in the root loop, traverse and increment the depth
     */
    for (auto successor : successorsToTraverse) {

      /*
       * Default the depth to max int if the block has never been reached yet
       */
      int successorDepth = INT32_MAX;
      if (blockToMinimumDepthMap.find(successor) != blockToMinimumDepthMap.end()) {
        successorDepth = blockToMinimumDepthMap.at(successor);
      }

      if (successorDepth > depth + 1) {
        blockToMinimumDepthMap.insert({successor, depth + 1});
        blockQueue.push(successor);
      }
    }
  }

  for (auto blockDepth : blockToMinimumDepthMap) {
    blockDepth.first->print(errs() << "Block with depth: " << blockDepth.second << "\n");
  }
  originalStoreDominatingBlock->print(errs() << "Original store dominating Block\n");
  for (auto frontierBlock : originalFrontierBlocks) {
    frontierBlock->print(errs() << "Frontier Block\n");
  }

  /*
   * Determine the minimum depth at which to collect blocks for the frontier
   * NOTE: Exclude the loop entry exit. In that special case, the value loaded
   * within the loop body does NOT necessarily reflect the last iteration's store
   * to the spill environment. A load in that exit block is necessary.
   */
  int minDepth = INT32_MAX;
  for (auto frontierBlock : originalFrontierBlocks) {
    assert(blockToMinimumDepthMap.find(frontierBlock) != blockToMinimumDepthMap.end());
    auto frontierDepth = blockToMinimumDepthMap.at(frontierBlock);
    minDepth = min(minDepth, frontierDepth);
  }
  for (auto edge : loopStructure->getLoopExitEdges()) {
    if (edge.first == loopHeader) continue;
    assert(blockToMinimumDepthMap.find(edge.first) != blockToMinimumDepthMap.end());
    auto loopInternalExitingDepth = blockToMinimumDepthMap.at(edge.first);
    minDepth = min(minDepth, loopInternalExitingDepth);
  }
  for (auto latch : loopStructure->getLatches()) {
    assert(blockToMinimumDepthMap.find(latch) != blockToMinimumDepthMap.end());
    auto latchDepth = blockToMinimumDepthMap.at(latch);
    minDepth = min(minDepth, latchDepth);
  }

  /*
   * Collect all blocks at the minimum depth
   */
  for (auto blockDepthPair : blockToMinimumDepthMap) {
    auto block = blockDepthPair.first;
    auto depth = blockDepthPair.second;
    if (depth != minDepth) continue;

    originalFrontierBlocks.insert(block);
  }

  /*
   * Optimization: remove any blocks in the frontier dominated by other blocks in the frontier
   */
  std::unordered_set<BasicBlock *> unnecessaryFrontierBlocks;
  for (auto block : originalFrontierBlocks) {
    for (auto otherBlock : originalFrontierBlocks) {
      if (block == otherBlock) continue;
      if (!originalLoopDS->DT.dominates(otherBlock, block)) continue;
      unnecessaryFrontierBlocks.insert(block);
      break;
    }
  }
  for (auto block : unnecessaryFrontierBlocks) {
    originalFrontierBlocks.erase(block);
  }

  // TODO: Traverse the loop and assert the header cannot be reached without passing through the frontier

  return;
}

void HELIX::replaceUsesOfSpilledPHIWithLoads (
  LoopDependenceInfo *LDI,
  std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
  SpilledLoopCarriedDependency *spill,
  Value *spillEnvPtr,
  DominatorSummary *originalLoopDS,
  std::unordered_set<BasicBlock *> &originalFrontierBlocks
) {

  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);

  /*
   * Insert a load in each frontier block, placed before any user/store in that block
   */
  std::unordered_set<User *> spillUsers{spill->loopCarriedPHI->user_begin(), spill->loopCarriedPHI->user_end()};
  for (auto originalBlock : originalFrontierBlocks) {
    auto cloneBlock = helixTask->getCloneOfOriginalBasicBlock(originalBlock);

    /*
     * Insert at the bottom of the block if no user or spill store are in the block
     * Otherwise, insert right before the first user/store
     */
    auto insertPoint = cloneBlock->getTerminator();
    for (auto &I : *cloneBlock) {
      auto isUserInst = spillUsers.find(&I) != spillUsers.end();
      auto store = dyn_cast<StoreInst>(&I);
      auto isStoreInst = store != nullptr
        && spill->environmentStores.find(store) != spill->environmentStores.end();
      if (!isUserInst && !isStoreInst) continue;
      insertPoint = &I;
      break;
    }

    IRBuilder<> spillValueBuilder(insertPoint);
    auto spillLoad = spillValueBuilder.CreateLoad(spillEnvPtr);
    spill->environmentLoads.insert(spillLoad);

    /*
     * Map uses for users that are dominated by this frontier block's load
     */
    for (auto user : spillUsers) {
      auto cloneUserBlock = cast<Instruction>(user)->getParent();
      auto originalUserBlock = cloneToOriginalBlockMap.at(cloneUserBlock);
      if (!originalLoopDS->DT.dominates(originalBlock, originalUserBlock)) continue;
      user->replaceUsesOfWith(spill->loopCarriedPHI, spillLoad);
    }
  }

  /*
   * Ensure no uses of the spilled PHI exist anymore
   */
  assert(spill->loopCarriedPHI->user_begin() == spill->loopCarriedPHI->user_end());

  return ;
}

std::unordered_map<BasicBlock *, Instruction *> HELIX::propagateLoadsOfSpilledLCDToLoopExits (
  LoopDependenceInfo *LDI,
  std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
  SpilledLoopCarriedDependency *spill,
  Value *spillEnvPtr
) {

  std::unordered_map<BasicBlock *, Instruction *> exitToPropagatedValueMap;

  /*
   * Collect spill loads on the frontier. They are the values
   * to be propagated to loop exits (EXCLUDING a loop entry block's exit) using PHINode instructions
   */
  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);
  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto clonedHeader = helixTask->getCloneOfOriginalBasicBlock(loopHeader);
  Type *valueType = nullptr;
  std::unordered_map<BasicBlock *, Value *> blockToPropagatedValueMap;
  std::queue<BasicBlock *> blockQueue;
  for (auto load : spill->environmentLoads) {
    auto loadBlock = load->getParent();
    blockToPropagatedValueMap.insert({loadBlock, load});
    blockQueue.push(loadBlock);
    valueType = load->getType();
  }

  /*
   * Propagate values through to exits (ignoring the loop entry and its possible exit)
   */
  while (!blockQueue.empty()) {
    auto block = blockQueue.front();
    blockQueue.pop();

    /*
     * If this block does NOT have a propagated value, aggregate one from predecessors
     */
    if (blockToPropagatedValueMap.find(block) == blockToPropagatedValueMap.end()) {

      /*
       * Consolidate predecessor propagated values
       * If only 1, do not create a trivial PHI
       */
      std::unordered_set<Value *> uniqueValues;
      std::unordered_map<BasicBlock *, Value *> predecessorToValueMap;
      for (auto predecessor : predecessors(block)) {
        auto predecessorValue = blockToPropagatedValueMap.at(predecessor);
        predecessorToValueMap.insert({predecessor, predecessorValue});
        uniqueValues.insert(predecessorValue);
      }

      if (uniqueValues.size() > 1) {
        IRBuilder<> builder(block->getFirstNonPHIOrDbgOrLifetime());
        auto phi = builder.CreatePHI(valueType, predecessorToValueMap.size());
        blockToPropagatedValueMap.insert({block, phi});
        for (auto predecessorAndValue : predecessorToValueMap) {
          phi->addIncoming(predecessorAndValue.second, predecessorAndValue.first);
        }
      } else {
        auto uniqueValue = *uniqueValues.begin();
        blockToPropagatedValueMap.insert({block, uniqueValue});
      }
    }

    /*
     * Only traverse successors for which all predecessors have been traversed
     * Do not traverse successors of exits
     */
    auto originalBlock = cloneToOriginalBlockMap.at(block);
    if (!loopStructure->isIncluded(originalBlock)) continue;
    for (auto successor : successors(block)) {

      /*
       * Do not traverse through latches back to the header
       */
      if (block == clonedHeader) continue;

      bool allPredecessorsTraversed = true;
      for (auto predecessor : predecessors(successor)) {
        if (blockToPropagatedValueMap.find(predecessor) == blockToPropagatedValueMap.end()) {
          allPredecessorsTraversed = false;
          break;
        }
      }
      if (!allPredecessorsTraversed) continue;

      blockQueue.push(successor);
    }
  }

  /*
   * If the loop entry has an exit and no load is present in the header,
   * a load in that exit is added
   */
  for (auto exitEdge : loopStructure->getLoopExitEdges()) {
    auto exitingBlock = helixTask->getCloneOfOriginalBasicBlock(exitEdge.first);
    auto exitBlock = helixTask->getCloneOfOriginalBasicBlock(exitEdge.second);
    Instruction * exitValue = nullptr;
    if (blockToPropagatedValueMap.find(exitBlock) != blockToPropagatedValueMap.end()) {
      exitValue = cast<Instruction>(blockToPropagatedValueMap.at(exitBlock));
    } else {
      assert(exitingBlock == clonedHeader);
      if (blockToPropagatedValueMap.find(clonedHeader) != blockToPropagatedValueMap.end()) {
        exitValue = cast<Instruction>(blockToPropagatedValueMap.at(clonedHeader));
      } else {
        IRBuilder<> exitBuilder(exitBlock->getFirstNonPHIOrDbgOrLifetime());
        exitValue = exitBuilder.CreateLoad(spillEnvPtr);
      }
    }

    exitToPropagatedValueMap.insert({ exitBlock, exitValue });
    exitBlock->print(errs() << "Exit block:\n");
    exitValue->print(errs() << "Exit value: "); errs() << "\n";
  }

  return exitToPropagatedValueMap;
}