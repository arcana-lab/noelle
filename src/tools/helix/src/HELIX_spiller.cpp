/*
 * Copyright 2016 - 2023  Angelo Matni, Simone Campanoni
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
#include "noelle/core/ReductionSCC.hpp"
#include "noelle/core/InductionVariableSCC.hpp"
#include "noelle/tools/HELIX.hpp"

namespace arcana::noelle {

void HELIX::spillLoopCarriedDataDependencies(LoopContent *LDI,
                                             DataFlowResult *reachabilityDFR,
                                             HELIXTask *helixTask) {

  /*
   * Fetch the header.
   */
  auto loopSummary = LDI->getLoopStructure();
  auto loopHeader = loopSummary->getHeader();
  auto loopPreHeader = loopSummary->getPreHeader();
  auto clonedPreheader = helixTask->getCloneOfOriginalBasicBlock(loopPreHeader);

  /*
   * Fetch the loop information.
   */
  auto loopFunction = loopSummary->getFunction();
  auto sccManager = LDI->getSCCManager();
  auto sccdag = sccManager->getSCCDAG();
  auto loopIVManager = LDI->getInductionVariableManager();

  /*
   * Collect all PHIs in the loop header; they are local variables
   * with loop carried data dependencies and need to be spilled
   * NOTE: There need not be a single loop carried PHI that needs spilling.
   * Non-independent function calls and already-in-memory data are such
   * examples.
   */
  std::vector<PHINode *> originalLoopCarriedPHIs;
  std::vector<PHINode *> clonedLoopCarriedPHIs;
  for (auto &phi : loopHeader->phis()) {
    auto phiSCC = sccdag->sccOfValue(cast<Value>(&phi));
    assert(phiSCC != nullptr);
    auto sccInfo = sccManager->getSCCAttrs(phiSCC);
    assert(sccInfo != nullptr);
    if (isa<ReductionSCC>(sccInfo) || isa<InductionVariableSCC>(sccInfo)
        || isa<PeriodicVariableSCC>(sccInfo)) {
      continue;
    }
    if (loopIVManager->doesContributeToComputeAnInductionVariable(&phi)) {
      continue;
    }
    errs() << this->prefixString << "    Spill " << phi << "\n";
    originalLoopCarriedPHIs.push_back(&phi);
    auto clonePHI = (PHINode *)(helixTask->getCloneOfOriginalInstruction(&phi));
    assert(clonePHI != nullptr);
    clonedLoopCarriedPHIs.push_back(clonePHI);
  }

  /*
   * Register each PHI as part of the loop carried environment
   */
  std::vector<Type *> phiTypes;
  std::set<uint32_t> nonReducablePHIs;
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
   * Create a new environment builder to pass the non-reducable PHIs, which will
   * spill to the stack of the caller of the HELIX task.
   */
  this->loopCarriedLoopEnvironmentBuilder =
      new LoopEnvironmentBuilder(this->noelle.getProgram()->getContext(),
                                 phiTypes,
                                 nonReducablePHIs,
                                 {},
                                 1,
                                 1);

  /*
   * Fetch the unique user of the environment builder dedicated to spilled
   * variables.
   */
  auto envUser = this->loopCarriedLoopEnvironmentBuilder->getUser(0);

  envUser->setEnvironmentArray(entryBuilder.CreateBitCast(
      helixTask->loopCarriedArrayArg,
      PointerType::getUnqual(
          loopCarriedLoopEnvironmentBuilder->getEnvironmentArrayType())));

  /*
   * Allocate the environment array (64 byte aligned)
   * Load incoming values from the preheader
   */
  IRBuilder<> loopFunctionBuilder(&*loopFunction->begin()->begin());
  loopCarriedLoopEnvironmentBuilder->allocateEnvironmentArray(
      loopFunctionBuilder);
  loopCarriedLoopEnvironmentBuilder->generateEnvVariables(loopFunctionBuilder);

  IRBuilder<> builder(this->entryPointOfParallelizedLoop);
  for (auto envVariableID = 0; envVariableID < originalLoopCarriedPHIs.size();
       ++envVariableID) {
    auto phi = originalLoopCarriedPHIs[envVariableID];
    auto preHeaderIndex = phi->getBasicBlockIndex(loopPreHeader);
    auto preHeaderV = phi->getIncomingValue(preHeaderIndex);
    builder.CreateStore(
        preHeaderV,
        loopCarriedLoopEnvironmentBuilder->getEnvironmentVariable(
            envVariableID));
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
    auto spilled = new SpilledLoopCarriedDependence(originalPHI, clonePHI);
    this->spills.insert(spilled);

    /*
     * Track the initial value of this spilled variable
     */
    spilled->clonedInitialValue =
        clonePHI->getIncomingValueForBlock(clonedPreheader);

    /*
     * Create GEP access of the environment variable at index i
     */
    auto envPtr = envUser->createEnvironmentVariablePointer(entryBuilder,
                                                            phiI,
                                                            phiTypes[phiI]);

    this->createLoadsAndStoresToSpilledLCD(LDI,
                                           reachabilityDFR,
                                           cloneToOriginalBlockMap,
                                           spilled,
                                           envPtr);
  }
}

void HELIX::createLoadsAndStoresToSpilledLCD(
    LoopContent *LDI,
    DataFlowResult *reachabilityDFR,
    std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
    SpilledLoopCarriedDependence *spill,
    Value *spillEnvPtr) {

  /*
   * Fetch task and loop
   */
  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);
  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto originalLoopFunction = loopHeader->getParent();
  auto DS = this->noelle.getDominators(originalLoopFunction);

  /*
   * Store loop carried dependencies into the spill environment
   * Identify the basic block dominating all stores to the spill environment
   */
  this->insertStoresToSpilledLCD(LDI,
                                 cloneToOriginalBlockMap,
                                 spill,
                                 spillEnvPtr);

  /*
   * Define a frontier across the loop extending out from users of the spill
   * This frontier will determine where to insert any needed loads
   * so that the value of the spill environment is known every iteration and
   * can be propagated to the header for potential use in the live out
   * environment
   */
  std::unordered_set<BasicBlock *> originalFrontierBlocks;
  this->defineFrontierForLoadsToSpilledLCD(LDI,
                                           reachabilityDFR,
                                           cloneToOriginalBlockMap,
                                           spill,
                                           DS,
                                           originalFrontierBlocks);

  this->replaceUsesOfSpilledPHIWithLoads(LDI,
                                         cloneToOriginalBlockMap,
                                         spill,
                                         spillEnvPtr,
                                         DS,
                                         originalFrontierBlocks);
}

void HELIX::insertStoresToSpilledLCD(
    LoopContent *LDI,
    std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
    SpilledLoopCarriedDependence *spill,
    Value *spillEnvPtr) {

  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);
  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto loopPreHeader = loopStructure->getPreHeader();
  auto headerClone = helixTask->getCloneOfOriginalBasicBlock(loopHeader);
  auto preHeaderClone = helixTask->getCloneOfOriginalBasicBlock(loopPreHeader);

  /*
   * Store loop carried values of the PHI into the environment
   */
  for (auto inInd = 0; inInd < spill->getClone()->getNumIncomingValues();
       ++inInd) {
    auto incomingBB = spill->getClone()->getIncomingBlock(inInd);
    if (incomingBB == preHeaderClone) {
      continue;
    }

    /*
     * Determine the position of the incoming value's producer
     * If it s an instruction computed within the loop, insert the store at that
     * point Otherwise, insert at the incoming block's entry
     */
    auto incomingV = spill->getClone()->getIncomingValue(inInd);
    Instruction *insertPoint = incomingBB->getTerminator();
    if (auto incomingI = dyn_cast<Instruction>(incomingV)) {
      auto blockOfIncomingI = incomingI->getParent();
      auto originalBlockOfIncomingI =
          cloneToOriginalBlockMap.at(blockOfIncomingI);
      if (loopStructure->isIncluded(originalBlockOfIncomingI)) {
        insertPoint = isa<PHINode>(incomingI)
                          ? blockOfIncomingI->getFirstNonPHIOrDbgOrLifetime()
                          : incomingI->getNextNode();
      }
    }

    IRBuilder<> builder(insertPoint);
    spill->environmentStores.insert(
        builder.CreateStore(incomingV, spillEnvPtr));
  }
}

void HELIX::defineFrontierForLoadsToSpilledLCD(
    LoopContent *LDI,
    DataFlowResult *reachabilityDFR,
    std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
    SpilledLoopCarriedDependence *spill,
    DominatorSummary *originalLoopDS,
    std::unordered_set<BasicBlock *> &originalFrontierBlocks) {
  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();

  /*
   * Compute all blocks where a load to the spill environment may be invalidated
   * This is every block which spill stores can reach within an iteration EXCEPT
   * for the store's block itself, as the load can be placed before the store in
   * that block
   *
   * NOTE: The reachability analysis was computed BEFORE the stores were added,
   * so to query the results, we use the terminator of the store' basic block
   */
  std::unordered_set<BasicBlock *> invalidatedBlocks;
  for (auto store : spill->environmentStores) {
    auto cloneBlock = store->getParent();
    auto originalBlock = cloneToOriginalBlockMap.at(cloneBlock);

    for (auto successorToStoreBlock : successors(originalBlock)) {

      /*
       * Ignore loop exits and the loop header
       */
      if (successorToStoreBlock == loopHeader)
        continue;
      if (!loopStructure->isIncluded(successorToStoreBlock))
        continue;

      /*
       * Track each loop internal successor block as being invalidated
       * Check its terminator's reachability
       * TODO: Find an easier way to isolate an instruction in this successor
       * block that is guaranteed NOT to be a store just inserted but has
       * instructions in this block in its OUT set so that we don't have to
       * individually add successors to the invalidated set
       */
      invalidatedBlocks.insert(successorToStoreBlock);
      auto terminator = successorToStoreBlock->getTerminator();

      for (auto reachableI : reachabilityDFR->OUT(terminator)) {
        if (!isa<Instruction>(reachableI)) {
          continue;
        }
        auto reachableBlock = cast<Instruction>(reachableI)->getParent();
        invalidatedBlocks.insert(reachableBlock);
      }
    }
  }

  /*
   * NOTE: The reachability analysis does not extend to loop exit blocks.
   * We therefore have to traverse loop exit edges to add invalidated exit
   * blocks
   */
  for (auto exitEdge : loopStructure->getLoopExitEdges()) {
    auto exitingBlock = exitEdge.first;
    if (invalidatedBlocks.find(exitingBlock) == invalidatedBlocks.end())
      continue;
    invalidatedBlocks.insert(exitEdge.second);
  }

  /*
   * Find the first dominator of the block NOT invalidated by stores
   * NOTE: The header will always be valid because
   * 1) if a store is in the header, only the header's successors are considered
   * invalidated 2) if the store is in any other block, the reachability within
   * one iteration does NOT extend back to the header
   */
  auto getValidBlockToLoadIn = [&](BasicBlock *startBlock) -> BasicBlock * {
    auto validBlock = startBlock;
    while (invalidatedBlocks.find(validBlock) != invalidatedBlocks.end()) {
      assert(validBlock != loopHeader);

      auto invalidatedNode = originalLoopDS->DT.getNode(validBlock);
      auto immediateDominatorNode = invalidatedNode->getIDom();
      validBlock = immediateDominatorNode->getBlock();
    }

    return validBlock;
  };

  /*
   * Traverse dominators of users of the spilled PHI until a block is found that
   * will not be invalidated by spill stores
   */
  for (auto user : spill->getClone()->users()) {
    auto userInst = cast<Instruction>(user);
    auto cloneUserBlock = userInst->getParent();
    auto originalUserBlock = cloneToOriginalBlockMap.at(cloneUserBlock);

    /*
     * If the user is a PHI, since a load cannot be placed before a PHI,
     * identify a strictly dominating block of the user
     */
    if (auto userPHI = dyn_cast<PHINode>(user)) {
      for (auto i = 0; i < userPHI->getNumIncomingValues(); ++i) {
        if (spill->getClone() != userPHI->getIncomingValue(i)) {
          continue;
        }
        auto cloneIncomingBlock = userPHI->getIncomingBlock(i);
        auto originalIncomingBlock =
            cloneToOriginalBlockMap.at(cloneIncomingBlock);
        originalUserBlock = originalLoopDS->DT.findNearestCommonDominator(
            originalUserBlock,
            originalIncomingBlock);
      }
    }

    /*
     * Find the first dominator of the user block NOT invalidated by stores
     */
    auto validBlock = getValidBlockToLoadIn(originalUserBlock);
    originalFrontierBlocks.insert(validBlock);
  }

  /*
   * For each loop exit, find the first block NOT invalidated by stores
   */
  for (auto exitBlock : loopStructure->getLoopExitBasicBlocks()) {
    auto validBlock = getValidBlockToLoadIn(exitBlock);
    originalFrontierBlocks.insert(validBlock);
  }

  /*
   * Optimization: remove any blocks in the frontier dominated by other blocks
   * in the frontier
   */
  std::unordered_set<BasicBlock *> unnecessaryFrontierBlocks;
  for (auto block : originalFrontierBlocks) {
    for (auto otherBlock : originalFrontierBlocks) {
      if (block == otherBlock)
        continue;
      if (!originalLoopDS->DT.dominates(otherBlock, block))
        continue;
      unnecessaryFrontierBlocks.insert(block);
      break;
    }
  }
  for (auto block : unnecessaryFrontierBlocks) {
    originalFrontierBlocks.erase(block);
  }

  return;
}

void HELIX::replaceUsesOfSpilledPHIWithLoads(
    LoopContent *LDI,
    std::unordered_map<BasicBlock *, BasicBlock *> &cloneToOriginalBlockMap,
    SpilledLoopCarriedDependence *spill,
    Value *spillEnvPtr,
    DominatorSummary *originalLoopDS,
    std::unordered_set<BasicBlock *> &originalFrontierBlocks) {

  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);
  auto loopStructure = LDI->getLoopStructure();

  /*
   * Insert a load in each frontier block, placed before any user/store in that
   * block
   */
  std::unordered_set<User *> spillUsers{ spill->getClone()->user_begin(),
                                         spill->getClone()->user_end() };
  for (auto originalBlock : originalFrontierBlocks) {
    auto cloneBlock = helixTask->getCloneOfOriginalBasicBlock(originalBlock);

    /*
     * Insert at the bottom of the block if no user or spill store are in the
     * block Otherwise, insert right before the first user/store
     */
    auto insertPoint = cloneBlock->getTerminator();
    auto I = cloneBlock->getFirstNonPHI();
    while (!I->isTerminator()) {
      auto isUserInst = spillUsers.find(I) != spillUsers.end();
      auto store = dyn_cast<StoreInst>(I);
      auto isStoreInst = store != nullptr
                         && spill->environmentStores.find(store)
                                != spill->environmentStores.end();
      if (!isUserInst && !isStoreInst) {
        I = I->getNextNode();
        continue;
      }
      insertPoint = I;
      break;
    }

    IRBuilder<> spillValueBuilder(insertPoint);
    auto spillLoad =
        spillValueBuilder.CreateLoad(spillEnvPtr,
                                     "noelle.helix.spilled_variable");
    spill->environmentLoads.insert(spillLoad);

    /*
     * Map uses for users that are dominated by this frontier block's load
     */
    for (auto user : spillUsers) {
      auto cloneUserBlock = cast<Instruction>(user)->getParent();
      auto originalUserBlock = cloneToOriginalBlockMap.at(cloneUserBlock);
      if (!originalLoopDS->DT.dominates(originalBlock, originalUserBlock))
        continue;
      user->replaceUsesOfWith(spill->getClone(), spillLoad);
    }

    /*
     * If this load dominates an exit block, mark it as a live out instruction
     */
    for (auto originalExit : loopStructure->getLoopExitBasicBlocks()) {
      if (!originalLoopDS->DT.dominates(originalBlock, originalExit))
        continue;
      helixTask->addLiveOut(spill->getOriginal(), spillLoad);
    }
  }

  /*
   * Ensure no uses of the spilled PHI exist anymore. Then erase the value
   */
  assert(spill->getClone()->user_begin() == spill->getClone()->user_end());
  spill->getClone()->eraseFromParent();
  helixTask->removeOriginalInstruction(spill->getOriginal());

  /*
   * Ensure all live out blocks have an available load
   */
  for (auto originalExit : loopStructure->getLoopExitBasicBlocks()) {
    bool isCovered = false;
    for (auto load : spill->environmentLoads) {
      auto cloneBlock = load->getParent();
      auto originalBlock = cloneToOriginalBlockMap.at(cloneBlock);
      if (!originalLoopDS->DT.dominates(originalBlock, originalExit))
        continue;

      isCovered = true;
      break;
    }
    assert(isCovered);
  }
}

} // namespace arcana::noelle
