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
  auto originalLoopFunction = loopHeader->getParent();
  DominatorTree originalLoopDT(*originalLoopFunction);
  PostDominatorTree originalLoopPDT(*originalLoopFunction);
  DominatorSummary DS(originalLoopDT, originalLoopPDT);
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
        : DS.DT.findNearestCommonDominator(originalStoreDominatingBlock, originalStoreBlock);
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
      : DS.DT.findNearestCommonDominator(originalStoreDominatingBlock, childLoopPreheader);
  }
  BasicBlock *cloneStoreDominatingBlock = helixTask->getCloneOfOriginalBasicBlock(originalStoreDominatingBlock);

  /*
   * Replace uses of PHI with environment loads
   * Determine which load is available upon exiting the loop
   * TODO: Improve determining how many loads are needed if there isn't just one exit from the header
   */
  IRBuilder<> headerBuilder(headerClone->getFirstNonPHIOrDbgOrLifetime());
  LoadInst * liveOutLoad = nullptr;
  auto loopExits = loopStructure->getLoopExitBasicBlocks();
  if (loopExits.size() > 1) {
    liveOutLoad = headerBuilder.CreateLoad(spillEnvPtr);

    std::unordered_set<User *> phiUsers{spill->loopCarriedPHI->user_begin(), spill->loopCarriedPHI->user_end()};
    for (auto user : phiUsers) {
      auto userInst = cast<Instruction>(user);
      userInst->replaceUsesOfWith(spill->loopCarriedPHI, liveOutLoad);
    }

  } else {

    /*
     * Identify basic blocks to add loads, tracking the uses of that load to be created
     */
    std::unordered_map<BasicBlock *, std::unordered_set<Instruction *>> blockToUserMap;
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
          originalUserBlock = DS.DT.findNearestCommonDominator(originalUserBlock, originalIncomingBlock);
        }
      }

      /*
       * Find nearest common dominator of user and store dominator
       */
      auto originalCommonDominatorBlock =
        DS.DT.findNearestCommonDominator(originalStoreDominatingBlock, originalUserBlock);
      auto cloneCommonDominatorBlock = helixTask->getCloneOfOriginalBasicBlock(originalCommonDominatorBlock);
      blockToUserMap[cloneCommonDominatorBlock].insert(userInst);
    }

    /*
     * Insert a load before the first user in each block requiring one
     */
    std::unordered_map<BasicBlock *, LoadInst *> blockToLoadMap;
    for (auto blockAndUsers : blockToUserMap) {
      auto block = blockAndUsers.first;
      auto users = blockAndUsers.second;

      /*
       * Insert at the bottom of the block if no user or spill store are in the block
       * Otherwise, insert right before the first user/store
       */
      auto insertPoint = block->getTerminator();
      for (auto &I : *block) {
        auto isUserInst = users.find(&I) != users.end();
        auto store = dyn_cast<StoreInst>(&I);
        auto isStoreInst = store != nullptr
          && spill->environmentStores.find(store) != spill->environmentStores.end();
        if (!isUserInst && !isStoreInst) continue;
        insertPoint = &I;
        break;
      }

      /*
       * Do not create duplicate loads in the same basic block
       * Ensure the load dominates all uses of it
       */
      IRBuilder<> spillValueBuilder(insertPoint);
      auto spillLoad = spillValueBuilder.CreateLoad(spillEnvPtr);
      blockToLoadMap.insert({block, spillLoad});
      spill->environmentLoads.insert(spillLoad);

      for (auto user : users) {
        user->replaceUsesOfWith(spill->loopCarriedPHI, spillLoad);
      }
    }

    /*
     * Insert or find a spill load dominating the exit block (and all stores to prevent
     * the load from not reflecting the loop entry PHI)
     */
    auto loopExitBlock = *loopExits.begin();
    auto dominatorOfExitAndStores =
      DS.DT.findNearestCommonDominator(originalStoreDominatingBlock, loopExitBlock);
    auto clonedDominator = helixTask->getCloneOfOriginalBasicBlock(dominatorOfExitAndStores);
    if (blockToLoadMap.find(clonedDominator) == blockToLoadMap.end()) {
      IRBuilder<> loopExitBuilder(clonedDominator->getFirstNonPHIOrDbgOrLifetime());
      liveOutLoad = loopExitBuilder.CreateLoad(spillEnvPtr);
    } else {
      liveOutLoad = blockToLoadMap.at(clonedDominator);
    }
  }

  /*
   * Translate instruction clone from cloned PHI to spilled load that is available
   * when exiting the loop
   */
  spill->environmentLoads.insert(liveOutLoad);
  helixTask->addInstruction(spill->originalLoopCarriedPHI, liveOutLoad);
  spill->loopCarriedPHI->eraseFromParent();

}
