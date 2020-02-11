/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopDistribution.hpp"

using namespace llvm;
 
bool LoopDistribution::splitLoop (
  LoopDependenceInfo const &LDI, 
  SCC *SCCToPullOut,
  std::set<Instruction *> &instructionsRemoved,
  std::set<Instruction *> &instructionsAdded
  ){
  std::set<SCC *> SCCs{};
  SCCs.insert(SCCToPullOut);
  auto modified = this->splitLoop(LDI, SCCs, instructionsRemoved, instructionsAdded);
  return modified;
}


bool LoopDistribution::splitLoop (
  LoopDependenceInfo const &LDI, 
  std::set<SCC *> const &SCCsToPullOut,
  std::set<Instruction *> &instructionsRemoved,
  std::set<Instruction *> &instructionsAdded
  ){
  std::set<Instruction *> Insts{};
  for (auto SCCToPullOut : SCCsToPullOut) {
    for (auto node : SCCToPullOut->getNodes()){
      auto v = node->getT();
      if (!isa<Instruction>(v)) {
        continue;
      }
      auto i = cast<Instruction>(v);
      Insts.insert(i);
    }
  }
  bool modified = this->splitLoop(LDI, Insts);
  return modified;
}


bool LoopDistribution::splitLoop (
  LoopDependenceInfo const &LDI,
  std::set<Instruction *> &instsToPullOut
  ){
  errs() << "LoopDistribution: Attempting Loop Distribution\n";
  for (auto inst : instsToPullOut) {
    errs() << "LoopDistribution: Asked to pull out " << *inst << "\n";
  }

  /*
   * Require that there is only one exit block. This simplifies how we deal with PHI nodes
   */
  if (LDI.numberOfExits() != 1) {
    errs() << "LoopDistribution: Abort: Number of exits is " << LDI.numberOfExits() << ", not 1\n";
    return false;
  }

  /*
   * Require that there is only one SCC with loop-carried control dependencies
   *   TODO(Lukas): Are we sure that multiple SCCs are bad?
   */
  auto controlSCCs = LDI.sccdagAttrs.getSCCsWithLoopCarriedControlDependencies();
  if (controlSCCs.size() != 1) {
    errs() << "LoopDistribution: Abort: Number of SCCs with loop-carried control dependencies is "
           << controlSCCs.size() << ", not 1\n";
    return false;
  }

  /*
   * Require that all SCCs that control the loop are cloneable and collect control instructions
   */
  std::set<Instruction *> controlInstructions{};
  for (auto controlSCC : controlSCCs) {
    if (LDI.sccdagAttrs.clonableSCCs.find(controlSCC) == LDI.sccdagAttrs.clonableSCCs.end()) {
      errs() << "LoopDistribution: Abort: Not all SCCs that control the loop are cloneable\n";
      return false;
    }
    for (auto pair : controlSCC->internalNodePairs()) {
      if (auto controlInst = dyn_cast<Instruction>(pair.second->getT())) {
        errs () << "Control SCC: " <<  *controlInst << "\n";
        controlInstructions.insert(controlInst);
      }
    }
  }

  /*
   * Remove control instructions from instsToPullOut
   */
  for (auto controlInst : controlInstructions) {
    if (instsToPullOut.erase(controlInst)) {
      errs() << "LoopDistribution: Removed " << *controlInst << " from instsToPullOut\n";
    }
  }

  /*
   * Require that there are no data dependencies between instsToPullOut and the rest of the loop
   */
  if (this->splitWouldRequireForwardingDataDependencies(LDI, instsToPullOut, controlInstructions)) {
    errs() << "LoopDistribution: Abort: Splitting the loop would require forwarding data dependencies\n";
    return false;
  }

  /*
   * Require that all instructions in instsToPullOut control-depend on the loop exiting block
   *   TODO(lukas): Ask if we are better off just checking if each instruction is in a loop BB
   */
  if (!this->allInstsToPullOutControlDependOnLoopExitingBlock(LDI, instsToPullOut)) {
    errs() << "LoopDistribution: Abort: Not all instructions control-depend on the loop exiting block\n";
    return false;
  }

  /*
   * Require that instsToPullOut and controlInstructions have no common instructions
   *   TODO(lukas): Decide if the API should be "ignore control instructions" or "reject them"
   */
  std::set<Instruction *> commonInstructions{};
  std::set_intersection(instsToPullOut.begin(), instsToPullOut.end(),
                        controlInstructions.begin(), controlInstructions.end(),
                        std::inserter(commonInstructions, commonInstructions.begin()));
  if (!commonInstructions.empty()) {
    errs() << "LoopDistribution: Abort: Asked to pull a control instruction out of the loop\n";
    return false;
  }

  /*
   * Splitting the loop is now safe
   */
  this->doSplit(LDI, instsToPullOut, controlInstructions);
  return true;
}


/*
 * Checks if any instructions in instsToPullOut are the source or destination of a data dependency
 *   to another instruction in the loop (external to instsToPullOut)
 */
bool LoopDistribution::splitWouldRequireForwardingDataDependencies (
  LoopDependenceInfo const &LDI,
  std::set<Instruction *> const &instsToPullOut,
  std::set<Instruction *> const &clonedInsts
  ){
  auto fn = [&LDI, &instsToPullOut, &clonedInsts](Value *toOrFrom, DataDependenceType ddType) -> bool {
    if (!isa<Instruction>(toOrFrom)) {
      return false;
    }
    auto i = cast<Instruction>(toOrFrom);

    /*
     * Ignore dependencies between instructions in we are pulling out or were already cloned
     */
    if (true
        && instsToPullOut.find(i) == instsToPullOut.end()
        && clonedInsts.find(i) == clonedInsts.end()
       ) {
      auto bb = i->getParent();

      /*
       * Only dependencies inside the loop should cause us to abort
       */
      if (std::find(LDI.loopBBs.begin(), LDI.loopBBs.end(), bb) != LDI.loopBBs.end()) {
        errs() << "LoopDistribution: Instruction "
               << *i << " is involved in a data dependency that would need to be forwarded\n";
        return true;
      }
    }
    return false;
  };
  auto pdg = LDI.getLoopDG();
  for (auto inst : instsToPullOut) {
    bool isSourceOfExternalDataDependency = pdg->iterateOverDependencesFrom(
      inst,
      false, // Control
      true,  // Memory
      true,  // Register
      fn
    );
    if (isSourceOfExternalDataDependency) {
      errs() << "LoopDistribution: Problem was dependency from " << *inst << "\n";
      return true;
    }
    bool isDestinationOfExternalDataDependency = pdg->iterateOverDependencesTo(
      inst,
      false, // Control
      true,  // Memory
      true,  // Register
      fn
    );
    if (isDestinationOfExternalDataDependency) {
      errs() << "LoopDistribution: Problem was dependency to " << *inst << "\n";
      return true;
    }
  }
  return false;
}


/*
 * Checks that all instructions in instsToPullOut control-depend on the loop exiting block
 *   TODO(Lukas): Requires that there only be one exit block
 */
bool LoopDistribution::allInstsToPullOutControlDependOnLoopExitingBlock (
  LoopDependenceInfo const &LDI,
  std::set<Instruction *> const &instsToPullOut
  ){

  /*
   * An exit block should have a single predecessor
   */
  auto loopExitingBlock = LDI.loopExitBlocks[0]->getSinglePredecessor();
  if (!loopExitingBlock) {
    errs() << "LoopDistribution: Exit block has more than one predecessor\n";
    return false;
  }

  /*
   * Get a set of the instructions that control-depend on the loop exit
   */
  std::set<Instruction *> controlDependsOnExit{};
  LDI.getLoopDG()->iterateOverDependencesFrom(
    loopExitingBlock->getTerminator(),
    true,  // Control
    false, // Memory
    false, // Register
    [&controlDependsOnExit](Value *toValue,
                            DataDependenceType ddType) -> bool {
      if (auto i = dyn_cast<Instruction>(toValue)) {
        controlDependsOnExit.insert(i);
      }
      return false;
    }
  );

  /*
   * Check if controlDependsOnExit is a superset of instsToPullOut
   */
  auto allControlDependOnExit =
    std::includes(controlDependsOnExit.begin(), controlDependsOnExit.end(),
                  instsToPullOut.begin(), instsToPullOut.end());
  return allControlDependOnExit;
}

void LoopDistribution::doSplit (
  LoopDependenceInfo const &LDI,
  std::set<Instruction *> const &instsToPullOut,
  std::set<Instruction *> const &controlInstructions
  ){
  errs() << "LoopDistribution: About to do split of " << *LDI.function << "\n";
  auto &cxt = LDI.function->getContext();

  /*
   * Duplicate the basic blocks of the loop and insert clones of all necessary
   *   non-branch instructions in order (instsToPullOut and controlInstructions)
   */
  std::unordered_map<Instruction *, Instruction *> instMap{};
  std::unordered_map<BasicBlock *, BasicBlock *> bbMap{};
  for (auto &BB : LDI.loopBBs) {
    auto cloneBB = BasicBlock::Create(cxt, "", LDI.function);
    bbMap[BB] = cloneBB;
    IRBuilder<> builder(cloneBB);
    for (auto &I : *BB) {
      if (isa<BranchInst>(I)) {
        continue;
      }
      if (false
          || instsToPullOut.find(&I) != instsToPullOut.end()
          || controlInstructions.find(&I) != controlInstructions.end()) {
        auto cloneInst = builder.Insert(I.clone());
        instMap[&I] = cloneInst;
      }
    }
  }
  errs() << "LoopDistribution: Finished cloning non-branch instructions\n";

  /*
   * Collect the branch instructions that can lead to exiting the loop. This needs to happen
   *   before we add branches to the new loop or getSinglePredecessor won't work
   */
  std::unordered_map<BasicBlock *, BranchInst *> oldExitBlockToBranch{};
  std::vector<BasicBlock *> oldExitingBlocks{};
  for (auto loopExitBlock : LDI.loopExitBlocks) {
    auto oldExitingBlock = loopExitBlock->getSinglePredecessor();
    oldExitingBlocks.push_back(oldExitingBlock);
    assert(oldExitingBlock && isa<BranchInst>(oldExitingBlock->getTerminator()));
    auto oldLoopExit = cast<BranchInst>(oldExitingBlock->getTerminator());
    oldExitBlockToBranch[loopExitBlock] = oldLoopExit;
  }
  errs() << "LoopDistribution: Finished collecting exit branches\n";

  /*
   * Map the original loop exit blocks to themselves so in the next section the new loop
   *   will have branches to the original exits
   */
  for (auto loopExitBlock : LDI.loopExitBlocks) {
    bbMap[loopExitBlock] = loopExitBlock;
  }

  /*
   * Duplicate all branch instructions (with correct successors).
   *   Cloned branches are not added to instMap because they don't produce values
   */
  for (auto &BB : LDI.loopBBs) {
    IRBuilder<> builder(bbMap.at(BB));
    auto terminator = BB->getTerminator();
    auto cloneTerminator = builder.Insert(terminator->clone());
    assert(isa<BranchInst>(terminator) && isa<BranchInst>(cloneTerminator));
    auto branch = cast<BranchInst>(terminator);
    auto cloneBranch = cast<BranchInst>(cloneTerminator);
    for (unsigned idx = 0; idx < branch->getNumSuccessors(); idx++) {
      auto oldBB = branch->getSuccessor(idx);
      auto newBB = bbMap.at(oldBB);
      cloneBranch->setSuccessor(idx, newBB);
    }
  }
  errs() << "LoopDistribution: Finished stitching together the new loop CFG\n";

  /*
   * Connect the original loop to the new loop using the branches we found earlier. This needs
   *   to happen after we add branches to the new loop or the branches we are about to add
   *   will mess up the process of stitching things together by pointing to blocks not in the map
   *   TODO(Lukas): This requires the one exit block assumption
   */
  auto newLoopHeader = bbMap.at(LDI.header);
  for (auto loopExitBlock : LDI.loopExitBlocks) {
    auto oldLoopExit = oldExitBlockToBranch[loopExitBlock];
    for (unsigned idx = 0; idx < oldLoopExit->getNumSuccessors(); idx++) {
      if (oldLoopExit->getSuccessor(idx) == loopExitBlock) {
        oldLoopExit->setSuccessor(idx, newLoopHeader);
        break;
      }
    }
  }
  errs() << "LoopDistribution: Finished connecting original loop to new loop\n";

  /*
   * Fix data flows for all instructions in the loop
   */
  for (auto &BB : LDI.loopBBs) {
    auto cloneBB = bbMap.at(BB);
    for (auto &cloneI : *cloneBB) {

      /*
       * Fix data flows that are values produced by instructions
       */
      for (unsigned idx = 0; idx < cloneI.getNumOperands(); idx++) {
        auto oldOperand = cloneI.getOperand(idx);
        if (auto oldInst = dyn_cast<Instruction>(oldOperand)) {
          auto it = instMap.find(oldInst);
          if (it != instMap.end()) {
            cloneI.setOperand(idx, it->second);
          }
        }
      }

      /*
       * Fix data flows that are incoming basic blocks in phi nodes. If the incoming block is
       *   the preheader of the original loop, replace it with the exiting block
       *   TODO(Lukas): This assumes only a single loop exit
       */
      if (auto clonePHI = dyn_cast<PHINode>(&cloneI)) {
        for (unsigned idx = 0; idx < clonePHI->getNumIncomingValues(); idx++) {
          auto oldBB = clonePHI->getIncomingBlock(idx);
          auto newBB = oldBB == LDI.preHeader
            ? oldExitingBlocks[0]
            : bbMap.at(oldBB);
          clonePHI->setIncomingBlock(idx, newBB);
        }
      }
    }
  }
  errs() << "LoopDistribution: Finished fixing instruction dependencies in the new loop\n";


  /*
   * Fix data flows for all instructions in exit blocks (only need to fix phi nodes)
   */
  for (auto loopExitBlock : LDI.loopExitBlocks) {
    for (auto &I : *loopExitBlock) {
      if (auto PHI = dyn_cast<PHINode>(&I)) {

        /*
         * There should only be one incoming basic block
         */
        assert(PHI->getNumIncomingValues() == 1);
        auto oldBB = PHI->getIncomingBlock(0);
        auto newBB = bbMap.at(oldBB);
        PHI->setIncomingBlock(0, newBB);
        auto oldValue = PHI->getIncomingValue(0);
        auto oldInst = cast<Instruction>(oldValue);
        auto it = instMap.find(oldInst);
        if (it != instMap.end()) {
          PHI->setOperand(0, it->second);
        }
      }
    }
  }
  errs() << "LoopDistribution: Finished fixing instruction dependencies in exit blocks\n";

  /*
   * Remove instructions from the original loop (if they are not control instructions)
   */
  for (auto inst : instsToPullOut) {
    if (controlInstructions.find(inst) == controlInstructions.end()) {
      inst->eraseFromParent();
    }
  }
  errs() << "LoopDistribution: Finished removing instructions from the original loop\n";

  /*
   * Remove instructions that are useless (we didn't need to clone them)
   */
  std::vector<Instruction *> uselessInstructions{};
  for (auto &BB : LDI.loopBBs) {
    auto cloneBB = bbMap.at(BB);
    for (auto &cloneI : *cloneBB) {
      if (cloneI.use_empty() && cloneI.isSafeToRemove()) {
        uselessInstructions.push_back(&cloneI);
        errs() << "LoopDistribution: Didn't need to clone " << cloneI << "\n";
        break;
      }
    }
  }
  /*
  for (auto uselessI : uselessInstructions) {
    uselessI->eraseFromParent();
  }
  */

  errs() << "LoopDistribution: Success: Finished split of " << *LDI.function << "\n";
  return ;
  }
