/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"
#include "HELIXTask.hpp"
#include "llvm/IR/Type.h"

namespace llvm::noelle {

void HELIX::rewireLoopForIVsToIterateNthIterations (LoopDependenceInfo *LDI) {

  /*
   * Fetch loop and IV information.
   */
  auto task = static_cast<HELIXTask *>(this->tasks[0]);
  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto loopPreHeader = loopStructure->getPreHeader();
  auto preheaderClone = task->getCloneOfOriginalBasicBlock(loopPreHeader);
  auto headerClone = task->getCloneOfOriginalBasicBlock(loopHeader);
  auto ivManager = LDI->getInductionVariableManager();

  auto entryTerminator = task->getEntry()->getTerminator();
  IRBuilder<> entryBuilder(entryTerminator);

  // TODO: Refactor this and DOALL's implementation of it into ParallelizationTechnique
  auto fetchClone = [&](Value *original) -> Value * {
    if (isa<ConstantData>(original)) return original;

    auto liveInClone = task->getCloneOfOriginalLiveIn(original);
    if (liveInClone) return liveInClone;

    assert(isa<Instruction>(original));
    auto originalI = cast<Instruction>(original);
    assert(task->isAnOriginalInstruction(originalI));
    return task->getCloneOfOriginalInstruction(originalI);
  };

  /*
   * There are situations where the SCC containing an IV is not deemed fully clonable,
   * so we spill those IVs. Skip those when re-wiring the step size of IVs
   */
  auto sccManager = LDI->getSCCManager();
  auto sccdag = sccManager->getSCCDAG();
  std::set<InductionVariable *> ivInfos;
  for (auto ivInfo : ivManager->getInductionVariables(*loopStructure)) {
    auto loopEntryPHI = ivInfo->getLoopEntryPHI();

    /*
     * FIXME: If we identify an IV as reducible, we produce a reducible execution
     * for it even if that is unnecessary. Fix that, then remove this check
     */ 
    auto scc = sccdag->sccOfValue(loopEntryPHI);
    auto sccInfo = sccManager->getSCCAttrs(scc);
    if (sccInfo->canExecuteReducibly()) continue;

    /*
     * If the instruction was spilled, it will not have a unique cloned instruction equivalent
     */
    if (!task->isAnOriginalInstruction(loopEntryPHI)) {
      continue;
    }

    ivInfos.insert(ivInfo);
  }

  /*
   * Collect clones of step size deriving values for all induction variables
   * of the top level loop
   */
  auto clonedStepSizeMap = cloneIVStepValueComputation(LDI, 0, entryBuilder);

  /*
   * Determine start value of the IV for the task
   * core_start: original_start + original_step_size * core_id
   */
  for (auto ivInfo : ivInfos) {
    auto startOfIV = fetchClone(ivInfo->getStartValue());
    auto stepOfIV = clonedStepSizeMap.at(ivInfo);
    auto originalIVPHI = ivInfo->getLoopEntryPHI();
    auto ivPHI = cast<PHINode>(fetchClone(originalIVPHI));

    auto offsetStartValue = IVUtility::computeInductionVariableValueForIteration(preheaderClone, ivPHI, startOfIV, stepOfIV, task->coreArg);
    ivPHI->setIncomingValueForBlock(preheaderClone, offsetStartValue);
  }

  /*
   * Determine additional step size to account for n cores each executing the task
   * jump_step_size: original_step_size * (num_cores - 1)
   */
  for (auto ivInfo : ivInfos) {
    auto stepOfIV = clonedStepSizeMap.at(ivInfo);
    auto originalIVPHI = ivInfo->getLoopEntryPHI();
    auto ivPHI = cast<PHINode>(fetchClone(originalIVPHI));

    Value* numCoresMinusOne = entryBuilder.CreateSub(
          task->numCoresArg,
          ConstantInt::get(task->numCoresArg->getType(), 1)
        );
    Value* jumpStepSize = IVUtility::scaleInductionVariableStep(preheaderClone, ivPHI, stepOfIV, numCoresMinusOne);

    IVUtility::stepInductionVariablePHI(preheaderClone, ivPHI, jumpStepSize);
  }

  /*
   * If the loop is governed by an IV, ensure that:
   * 1) the condition catches iterating past the exit value
   * 2) all non-clonable instructions in the header instead execute in the body and after exiting the loop
   */
  auto loopGoverningIVAttr = LDI->getLoopGoverningIVAttribution();
  if (!loopGoverningIVAttr) {

    /*
     * The loop is not governed by an IV.
     */
    return ;
  }

  /*
   * The loop is governed by an IV.
   *
   * The exit condition needs to be made non-strict to catch iterating past it
   */
  LoopGoverningIVUtility ivUtility(loopStructure, *ivManager, *loopGoverningIVAttr);
  auto originalCmpInst = loopGoverningIVAttr->getHeaderCompareInstructionToComputeExitCondition();
  auto originalBrInst = loopGoverningIVAttr->getHeaderBrInst();
  auto cmpInst = cast<CmpInst>(task->getCloneOfOriginalInstruction(originalCmpInst));
  auto brInst = cast<BranchInst>(task->getCloneOfOriginalInstruction(originalBrInst));
  auto originalHeaderExit = loopGoverningIVAttr->getExitBlockFromHeader();
  auto cloneHeaderExit = task->getCloneOfOriginalBasicBlock(originalHeaderExit);
  ivUtility.updateConditionAndBranchToCatchIteratingPastExitValue(cmpInst, brInst, cloneHeaderExit);
  auto updatedCmpInst = cmpInst;

  /*
   * TODO describe what the next code does
   */
  auto &loopGoverningIV = loopGoverningIVAttr->getInductionVariable();
  auto originalGoverningPHI = loopGoverningIV.getLoopEntryPHI();
  assert(originalGoverningPHI != nullptr);
  auto cloneGoverningPHI = task->getCloneOfOriginalInstruction(originalGoverningPHI);
  assert(cloneGoverningPHI != nullptr);
  auto origValueUsedToCompareAgainstExitConditionValue = loopGoverningIVAttr->getValueToCompareAgainstExitConditionValue();
  assert(origValueUsedToCompareAgainstExitConditionValue != nullptr);
  auto valueUsedToCompareAgainstExitConditionValue = task->getCloneOfOriginalInstruction(origValueUsedToCompareAgainstExitConditionValue);
  assert(valueUsedToCompareAgainstExitConditionValue != nullptr);
  auto updatedBrInst = brInst;
  auto headerSuccTrue = updatedBrInst->getSuccessor(0);
  auto headerSuccFalse = updatedBrInst->getSuccessor(1);
  auto isTrueExiting = headerSuccTrue == cloneHeaderExit;
  auto entryIntoBody = isTrueExiting ? headerSuccFalse : headerSuccTrue;

  /*
   * NOTE: When loop governing IV attribution allows for any other instructions in the header
   * other than those of the IV and its comparison, those unrelated instructions should be
   * copied into the body and the exit block (to preserve the number of times they execute)
   * 
   * The logic in the exit block must be guarded so only the "last" iteration executes it,
   * not any cores that pass the last iteration. This is further complicated because the mapping
   * of live-out environment producing instructions might need to be updated with the peeled
   * instructions in the exit block
   * 
   * A temporary mitigation is to transform loop latches with conditional branches that
   * verify if the next iteration would ever occur. This still requires live outs to be propagated
   * from both the header and the latches
   */

  /*
   * Collect instructions that can stay in the new header.
   */
  std::set<Instruction *> cloneInstsThatCanStayInTheNewHeader;
  std::vector<Instruction *> originalInstsThatCanStayInTheNewHeader;
  std::vector<Instruction *> originalInstsThatMustMove;
  for (auto &I : *loopHeader) {

    /*
     * Fetch the clone
     */
    auto cloneI = task->getCloneOfOriginalInstruction(&I);
    if (cloneI == nullptr){
      originalInstsThatMustMove.push_back(&I);
      continue ;
    }

    /*
     * Fetch the SCC that contains I (if it exists)
     */
    auto scc = sccdag->sccOfValue(&I);
    auto sccInfo = sccManager->getSCCAttrs(scc);
    auto sccType = sccInfo->getType();

    /*
     * Ensure the original instruction was not independent, not a PHI, not clonable
     * and not part of this loop governing IV attribution
     * 
     * HACK: We don't have a way to ask if an instruction is repeatable, so to be safe,
     * anything that isn't belonging to an IV is duplicated
     */
    if (isa<PHINode>(&I)) {
      originalInstsThatCanStayInTheNewHeader.push_back(&I);
      cloneInstsThatCanStayInTheNewHeader.insert(cloneI);
      continue;
    }
    if (  false
          || (originalCmpInst == &I)
          || (originalBrInst == &I)
      ){
      originalInstsThatCanStayInTheNewHeader.push_back(&I);
      cloneInstsThatCanStayInTheNewHeader.insert(cloneI);
      continue;
    }
    if (sccInfo->isInductionVariableSCC()) {
      originalInstsThatCanStayInTheNewHeader.push_back(&I);
      cloneInstsThatCanStayInTheNewHeader.insert(cloneI);
      continue;
    }

    /*
     * The clone of this original instruction must move out from the new header.
     */
    originalInstsThatMustMove.push_back(&I);
  }

  /*
   * Collect the instruction in the old header of the task that must move.
   */
  std::vector<Instruction *> cloneInstsThatMustMove;
  for (auto &I : *headerClone){
    if (cloneInstsThatCanStayInTheNewHeader.find(&I) != cloneInstsThatCanStayInTheNewHeader.end()){
      continue ;
    }

    /*
     * The instruction must move from the old header to the new one.
     */
    cloneInstsThatMustMove.push_back(&I);
  }

  /*
   * Move those instructions to the loop body (right at the beginning, in order)
   */
  auto firstBodyInst = entryIntoBody->getFirstNonPHIOrDbgOrLifetime();
  for (auto iIter = cloneInstsThatMustMove.rbegin(); iIter != cloneInstsThatMustMove.rend(); ++iIter) {
    auto cloneI = *iIter;
    cloneI->moveBefore(firstBodyInst);
    firstBodyInst = cloneI;
  }

  auto taskFunction = task->getTaskBody();
  auto &cxt = taskFunction->getContext();
  auto checkForLastExecutionBlock = BasicBlock::Create(cxt, "check_if_last_iteration_is_missing", taskFunction);
  this->lastIterationExecutionBlock = BasicBlock::Create(cxt, "last_iteration", taskFunction);
  IRBuilder<> lastIterationExecutionBuilder(this->lastIterationExecutionBlock);

  /*
   * Clone these instructions and execute them after exiting the loop ONLY IF
   * the previous iteration's IV value passes the loop guard.
   */
  std::vector<Instruction *> duplicatesInLastIterationBlock;
  std::map<Instruction *, Instruction *> duplicateOfTaskInst;
  for (auto cloneI : cloneInstsThatMustMove){

    /*
     * Fetch the original instruction of the current instruction in the task (if it exists)
     */
    auto originalI = task->getOriginalInstructionOfClone(cloneI);

    /*
     * Clone the task instruction
     */
    auto duplicateI = cloneI->clone();
    duplicateOfTaskInst[cloneI] = duplicateI;

    /*
     * Add the clone to the last-iteration basic block
     */
    lastIterationExecutionBuilder.Insert(duplicateI);

    /*
     * Keep track of the clone
     */
    duplicatesInLastIterationBlock.push_back(duplicateI);

    /*
     * Keep the map originalI <-> duplicate if originalI exists
     */
    if (originalI){
      this->lastIterationExecutionDuplicateMap.insert(std::make_pair(originalI, duplicateI));
    }
  }

  /*
   * Re-wire the cloned last execution instructions together
   */
  for (auto duplicateInst : duplicatesInLastIterationBlock){
    for (auto currentTaskInst : cloneInstsThatMustMove){
      auto duplicateOfCurrentTaskInst = duplicateOfTaskInst[currentTaskInst];
      duplicateInst->replaceUsesOfWith(currentTaskInst, duplicateOfCurrentTaskInst);
    }
  }

  /*
   * Fix the control flows
   */
  lastIterationExecutionBuilder.CreateBr(cloneHeaderExit);
  updatedBrInst->replaceSuccessorWith(cloneHeaderExit, checkForLastExecutionBlock);
  IRBuilder<> checkForLastExecutionBuilder(checkForLastExecutionBlock);

  /*
   * Compute the loop governing IV's value the previous iteration
   * (regardless of what core it would have executed on)
   */
  auto stepSize = clonedStepSizeMap.at(&loopGoverningIV);

  /*
   * Guard against this previous iteration.
   * If the condition would have exited the loop, skip the last execution block
   * If not, this was the last execution of the header
   *
   * Clone the original compare instruction of the loop governing IV.
   */
  auto prevIterGuard = cast<CmpInst>(updatedCmpInst->clone());

  /*
   * Make the predicate strict of the comparison instruction.
   *
   * NOTE: This is important if the original comparison was "== N" and the loop governing IV was a decreasing IV that stopped at N.
   * In this case, the comparison has been translated into "<= N" to catch past-last-iteration iterations. 
   * So, if we want to know whether we are the thread that executed the last iteration, then the comparison instruction that we must use is "< N" and if this returns true, then we are not the thread that executed the last iteration.
   */
  ivUtility.updateConditionToCheckIfWeHavePastExitValue(prevIterGuard);
  auto prevIterationValue = ivUtility.generateCodeToComputeValueToUseForAnIterationAgo(checkForLastExecutionBuilder, cloneGoverningPHI, stepSize);
  prevIterGuard->replaceUsesOfWith(valueUsedToCompareAgainstExitConditionValue, prevIterationValue);
  checkForLastExecutionBuilder.Insert(prevIterGuard);
  auto prevIterGuardTrueSucc = isTrueExiting ? cloneHeaderExit : this->lastIterationExecutionBlock;
  auto prevIterGuardFalseSucc = isTrueExiting ? this->lastIterationExecutionBlock : cloneHeaderExit;
  checkForLastExecutionBuilder.CreateCondBr(prevIterGuard, prevIterGuardTrueSucc, prevIterGuardFalseSucc);

  /*
   * Track duplicated live out values properly
   * This has to happen because we duplicated logic.
   * 
   * The correct live out for non-reducible live outs is simply the duplicated value
   * The correct live out for reducible live outs is EITHER:
   * 1) the duplicated value within the last iteration block
   * 2) the original value moved to the body from the previous iteration executed on this core
   * 
   * NOTE: Helix only has one task, as each core executes the same task
   */
  IRBuilder<> cloneHeaderExitBuilder(cloneHeaderExit->getFirstNonPHI());
  auto envUser = this->envBuilder->getUser(0);

  for (auto envIndex : envUser->getEnvIndicesOfLiveOutVars()) {

    /*
     * Only work with duplicated producers
     */
    auto originalProducer = (Instruction*)LDI->getEnvironment()->producerAt(envIndex);
    if (this->lastIterationExecutionDuplicateMap.find(originalProducer) == this->lastIterationExecutionDuplicateMap.end()) continue;

    /*
     * If the producer isn't reducible, simply mapping to the duplicated value is sufficient,
     * which is already done (stored in lastIterationExecutionDuplicateMap)
     */
    auto isReduced = this->envBuilder->isVariableReducable(envIndex);
    if (!isReduced) {
      continue;
    }

    /*
     * We need a PHI after the last iteration block to track whether this core will
     * store an intermediate of this reduced live out of the last iteration's value of it
     */
    auto originalIntermedateInHeader = this->fetchLoopEntryPHIOfProducer(LDI, originalProducer);
    auto cloneIntermediateInHeader = task->getCloneOfOriginalInstruction(originalIntermedateInHeader);
    auto duplicateProducerInLastIterationBlock = this->lastIterationExecutionDuplicateMap.at(originalProducer);
    auto producerType = originalProducer->getType();

    /*
     * Create a PHI, recieving the propagated body value if the last iteration didn't execute on this core,
     * and receiving the last iteration value if the last iteration did execute on this core
     * 
     * NOTE: We don't use the value moved to the body; that would not dominate this PHI. We use the
     * PHI that propagates that value, for which there is one because this is a reducible live out
     */
    auto phi = cloneHeaderExitBuilder.CreatePHI(producerType, 2);
    phi->addIncoming(cloneIntermediateInHeader, checkForLastExecutionBlock);
    phi->addIncoming(duplicateProducerInLastIterationBlock, lastIterationExecutionBlock);

    /*
     * Map from the original value of this producer to the PHI tracking the last value of this producer
     * NOTE: This is needed later when storing live outs
     */
    this->lastIterationExecutionDuplicateMap.erase(originalProducer);
    this->lastIterationExecutionDuplicateMap.insert(std::make_pair(originalProducer, phi));
  }

  return ;
}

}
