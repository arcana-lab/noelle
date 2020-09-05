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
#include "Architecture.hpp"

using namespace llvm ;

void HELIX::addSynchronizations (
  LoopDependenceInfo *LDI,
  std::vector<SequentialSegment *> *sss
  ){

  if (sss->size() == 0) {
    if (this->verbose != Verbosity::Disabled) {
      errs() << "HELIX: no sequential segments, skipping synchronization\n";
    }
    return;
  }

  assert(this->tasks.size() == 1);
  auto helixTask = static_cast<HELIXTask *>(this->tasks[0]);
  IRBuilder<> entryBuilder(helixTask->getEntry()->getTerminator());

  /*
   * Fetch the header.
   */
  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();

  /*
   * Fetch the loop function.
   */
  auto loopFunction = loopStructure->getFunction();
  auto &cxt = loopFunction->getContext();
  auto int64 = IntegerType::get(cxt, 64);

  /*
   * HACK: Fetch the first sequential segment instructions that can be entered
   * This is necessary because we do not re-order instructions not dependent on
   * each other to ensure sequential segments do not overlap
   */
  DominatorTree DT(*loopFunction);
  PostDominatorTree PDT(*loopFunction);
  DominatorSummary DS(DT, PDT);

  /*
   * Optimization: If the preamble SCC is not part of a sequential segment,
   * then determining whether the loop exited does not need to be synchronized
   */
  auto loopSCCDAG = LDI->sccdagAttrs.getSCCDAG();
  auto preambleSCCNodes = loopSCCDAG->getTopLevelNodes();
  assert(preambleSCCNodes.size() == 1 && "The loop internal SCCDAG should only have one preamble");
  auto preambleSCC = (*preambleSCCNodes.begin())->getT();
  // preambleSCC->printMinimal(errs() << "Preamble SCC:\n");
  SequentialSegment *preambleSS = nullptr;
  for (auto ss : *sss){
    for (auto scc : ss->getSCCs()) {
      if (scc == preambleSCC) {
        preambleSS = ss;
        break;
        // errs() << "Preamble is included\n";
      }
    }
    if (preambleSS != nullptr) break;
  }

  /*
   * Define a helper to fetch the appropriate ss entry in synchronization arrays
   */
  auto fetchEntry = [&entryBuilder, int64](Value *ssArray, int32_t ssID) -> Value * {

    /*
     * Compute the offset of the sequential segment entry.
     */
    auto ssOffset = ssID * Architecture::getCacheLineBytes();

    /*
     * Fetch the pointer to the sequential segment entry.
     */
    auto ssArrayAsInt = entryBuilder.CreatePtrToInt(ssArray, int64);
    auto ssEntryAsInt = entryBuilder.CreateAdd(ConstantInt::get(int64, ssOffset), ssArrayAsInt);
    return entryBuilder.CreateIntToPtr(ssEntryAsInt, ssArray->getType());
  };

  /*
   * Fetch sequential segments entry in the past and future array
   * Allocate space to track sequential segment entry state
   */
  std::vector<Value *> ssPastPtrs{}, ssFuturePtrs{}, ssStates{};
  for (auto ss : *sss) {
    ssPastPtrs.push_back(fetchEntry(helixTask->ssPastArrayArg, ss->getID()));
    ssFuturePtrs.push_back(fetchEntry(helixTask->ssFutureArrayArg, ss->getID()));

    /*
     * We must execute exactly one wait instruction for each sequential segment, for each loop iteration, and for each thread.
     *
     * Create a new variable at the beginning of the iteration.
     * We call this new variable, ssState.
     * This new variable is reponsible to store the information about whether a wait instruction of the current sequential segment has already been executed in the current iteration for the current thread.
     */
    auto ssStateAlloca = entryBuilder.CreateAlloca(int64);
    ssStateAlloca->moveBefore(helixTask->getEntry()->getFirstNonPHIOrDbgOrLifetime());
    ssStates.push_back(ssStateAlloca);
  }

  /*
   * Define the code that inject wait instructions.
   */
  auto injectWait = [&](SequentialSegment *ss, Instruction *justAfterEntry) -> void {

    /*
     * Separate out the basic block into 2 halves, the second starting with justAfterEntry
     */
    auto beforeEntryBB = justAfterEntry->getParent();
    auto ssEntryBBName = "SS" + std::to_string(ss->getID()) + "-entry";
    auto ssEntryBB = BasicBlock::Create(cxt, ssEntryBBName, helixTask->getTaskBody());
    IRBuilder<> ssEntryBuilder(ssEntryBB);
    auto afterEntry = justAfterEntry;
    while (afterEntry) {
      auto currentEntry = afterEntry;
      afterEntry = afterEntry->getNextNode();
      currentEntry->removeFromParent();
      ssEntryBuilder.Insert(currentEntry);
    }

    /*
     * Redirect PHI node incoming blocks in successors to beforeEntryBB so they are successors of ssEntryBB
     */
    for (auto succToEntry : successors(ssEntryBB)) {
      for (auto &phi : succToEntry->phis()) {
        auto incomingIndex = phi.getBasicBlockIndex(beforeEntryBB);
        phi.setIncomingBlock(incomingIndex, ssEntryBB);
      }
    }

    /*
     * Inject a call to HELIX_wait just before "justAfterEntry"
     * Set the ssState just after the call to HELIX_wait.
     * This will keep track of the fact that we have executed wait for ss in the current iteration.
     */
    auto ssWaitBBName = "SS" + std::to_string(ss->getID()) + "-wait";
    auto ssWaitBB = BasicBlock::Create(cxt, ssWaitBBName, helixTask->getTaskBody());
    IRBuilder<> ssWaitBuilder(ssWaitBB);
    auto wait = ssWaitBuilder.CreateCall(this->waitSSCall, { ssPastPtrs.at(ss->getID()) });
    auto ssState = ssStates.at(ss->getID());
    ssWaitBuilder.CreateStore(ConstantInt::get(int64, 1), ssState);
    ssWaitBuilder.CreateBr(ssEntryBB);

    /*
     * Check if the ssState has been set already.
     * If it did, then we have already executed the wait to enter this ss and must not invoke it again.
     * If it didn't, then we need to invoke HELIX_wait.
     */
    IRBuilder<> beforeEntryBuilder(beforeEntryBB);
    auto ssStateLoad = beforeEntryBuilder.CreateLoad(ssState);
    auto needToWait = beforeEntryBuilder.CreateICmpEQ(ssStateLoad, ConstantInt::get(int64, 0));
    beforeEntryBuilder.CreateCondBr(needToWait, ssWaitBB, ssEntryBB);

    /*
     * Track the call to wait
     */
    helixTask->waits.insert(cast<CallInst>(wait));
  };

  /*
   * Define the code that inject wait instructions.
   */
  auto injectSignal = [&](SequentialSegment *ss, Instruction *justBeforeExit) -> void {

    /*
     * Inject a call to HELIX_signal just after "justBeforeExit" 
     * NOTE: If the exit is not an unconditional branch, inject the signal in every successor block
     */
    auto block = justBeforeExit->getParent();
    auto terminator = block->getTerminator();
    auto justBeforeExitBr = dyn_cast<BranchInst>(justBeforeExit);
    if (!justBeforeExitBr || justBeforeExitBr->isUnconditional()) {
      Instruction *insertPoint = terminator == justBeforeExit ? terminator : justBeforeExit->getNextNode();
      IRBuilder<> beforeExitBuilder(insertPoint);
      auto signal = beforeExitBuilder.CreateCall(this->signalSSCall, { ssFuturePtrs.at(ss->getID()) });
      helixTask->signals.insert(cast<CallInst>(signal));
      return;
    }

    for (auto successorBlock : successors(block)) {
      IRBuilder<> beforeExitBuilder(successorBlock->getFirstNonPHIOrDbgOrLifetime());
      auto signal = beforeExitBuilder.CreateCall(this->signalSSCall, { ssFuturePtrs.at(ss->getID()) });
      helixTask->signals.insert(cast<CallInst>(signal));
    }
  };

  /*
   * On finishing the task, set the loop-is-over flag to true.
   */
  auto injectExitFlagSet = [&](Instruction *exitInstruction) -> void {
    IRBuilder<> setFlagBuilder(exitInstruction);
    setFlagBuilder.CreateStore(
      ConstantInt::get(int64, 1),
      helixTask->loopIsOverFlagArg
    );
  };

  /*
   * For each loop exit, ensure all other execution of all other sequential segments
   * is completed (by inserting waits) and then signal to the next core right before exiting
   *
   * NOTE: This is needed if live outs are being loaded from the loop carried environment
   * before being stored in the live out environment. Since we do not store to the live out
   * environment every iteration of the loop, this synchronization upon exiting is necessary
   */
  for (auto i = 0; i < helixTask->getNumberOfLastBlocks(); ++i) {
    auto loopExitBlock = helixTask->getLastBlock(i);
    auto loopExitTerminator = loopExitBlock->getTerminator();
    for (auto ss : *sss) {
      injectWait(ss, loopExitBlock->getFirstNonPHI());
      injectSignal(ss, loopExitTerminator);
    }
  }

  /*
   * Inject a check for whether the loop-is-over flag is true
   * Exit the loop if so, signaling preamble SS synchronization to avoid deadlock
   */
  auto injectExitFlagCheck = [&](Instruction *justAfterEntry) -> void {

    auto beforeCheckBB = justAfterEntry->getParent();
    auto afterCheckBB = BasicBlock::Create(cxt, "SS-passed-checkexit", loopFunction);
    auto failedCheckBB = BasicBlock::Create(cxt, "SS-failed-checkexit", loopFunction);

    IRBuilder<> afterCheckBuilder(afterCheckBB);
    auto afterEntry = justAfterEntry;
    while (afterEntry) {
      auto currentEntry = afterEntry;
      afterEntry = afterEntry->getNextNode();
      currentEntry->removeFromParent();
      afterCheckBuilder.Insert(currentEntry);
    }

    /*
     * Redirect PHI node incoming blocks in successors to the original basic block so they are successors
     * of the basic block after checking to exit
     */
    for (auto succToEntry : successors(afterCheckBB)) {
      for (auto &phi : succToEntry->phis()) {
        auto incomingIndex = phi.getBasicBlockIndex(beforeCheckBB);
        phi.setIncomingBlock(incomingIndex, afterCheckBB);
      }
    }

    IRBuilder<> checkFlagBuilder(beforeCheckBB);
    auto flagValue = checkFlagBuilder.CreateLoad(helixTask->loopIsOverFlagArg);
    auto isFlagSet = checkFlagBuilder.CreateICmpEQ(ConstantInt::get(int64, 1), flagValue);
    checkFlagBuilder.CreateCondBr(isFlagSet, failedCheckBB, afterCheckBB);

    IRBuilder<> failedCheckBuilder(failedCheckBB);
    auto brToExit = failedCheckBuilder.CreateBr(helixTask->getExit());
    for (auto ss : *sss) injectSignal(ss, brToExit);
  };

  /*
   * Once the preamble has been synchronized, if that was necessary, synchronize each sequential segment
   */
  for (auto ss : *sss){

    /*
     * Reset the value of ssState at the beginning of the iteration
     * NOTE: This has to be done BEFORE any preamble synchronization, so this
     * insertion comes after the check exit logic has already been inserted
     */
    auto firstLoopInst = loopHeader->getFirstNonPHIOrDbgOrLifetime();
    IRBuilder<> headerBuilder(firstLoopInst);
    headerBuilder.CreateStore(ConstantInt::get(int64, 0), ssStates.at(ss->getID()));

    /*
     * Inject waits.
     *
     * NOTE: If this is the preamble, simply insert the wait at the entry to the loop
     * Also inject an exit flag check for the preamble (AFTER the wait so the check is synchronized)
     */
    if (preambleSS != ss) {
      ss->forEachEntry([preambleSS, ss, &injectWait, &injectExitFlagCheck](Instruction *justAfterEntry) -> void {
        injectWait(ss, justAfterEntry);
      });
    } else {
      injectWait(ss, firstLoopInst);
      injectExitFlagCheck(firstLoopInst);
    }

    /*
     * NOTE: To prevent double counting successor blocks for signals,
     * when the exit is a conditional terminator, add the first instruction in all successors
     * to a set of all exits; then signal at all unique exits determined
     */
    std::unordered_set<Instruction *> exits;
    ss->forEachExit([&exits](Instruction *justBeforeExit) -> void {
      auto block = justBeforeExit->getParent();
      auto terminator = block->getTerminator();
      if (terminator != justBeforeExit || terminator->getNumSuccessors() == 1) {
        exits.insert(justBeforeExit);
        return ;
      }

      for (auto successor : successors(block)) {
        auto beginningOfSuccessor = successor->getFirstNonPHIOrDbgOrLifetime();
        exits.insert(beginningOfSuccessor);
      }
    });

    /*
     * NOTE: If this is the preamble, also insert signals after all loop exits
     */
    if (preambleSS == ss) {
      for (auto exitBlock : loopStructure->getLoopExitBasicBlocks()) {
        auto beginningOfExitBlock = exitBlock->getFirstNonPHIOrDbgOrLifetime();
        exits.insert(beginningOfExitBlock);
      }
    }

    /*
     * Inject signals at sequential segment exits
     *
     * NOTE: For the preamble, jnject the exit flag set after injecting the signal
     * so that the set instruction is placed before the signal call
     */
    for (auto exit : exits) {
      injectSignal(ss, exit);
      if (preambleSS == ss &&
        !loopStructure->isIncluded(exit)) {
        injectExitFlagSet(exit);
      }
    }
  }

  return ;
}
