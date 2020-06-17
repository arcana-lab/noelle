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
    errs() << "LoopDistribution: New Control SCC\n";
    for (auto pair : controlSCC->internalNodePairs()) {
      if (auto controlInst = dyn_cast<Instruction>(pair.second->getT())) {
        if (controlInst->mayHaveSideEffects()){
          errs() << "LoopDistribution: Abort: " << *controlInst << " is not clonable\n";
          return false;
        }
        errs () << "LoopDistribution: Control SCC: " <<  *controlInst << "\n";
        controlInstructions.insert(controlInst);
      }
    }
    /*
    if (LDI.sccdagAttrs.clonableSCCs.find(controlSCC) == LDI.sccdagAttrs.clonableSCCs.end()) {
      errs() << "LoopDistribution: Abort: Not all SCCs that control the loop are cloneable\n";
      return false;
    }
    */
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
   * Require that all instructions in instsToPullOut control-depend on a loop exiting block
   *   TODO(lukas): Ask if we are better off just checking if each instruction is in a loop BB
   */
  if (!this->allInstsToPullOutControlDependOnLoopExitingBlock(LDI, instsToPullOut)) {
    errs() << "LoopDistribution: Abort: Not all instructions control-depend on a loop exiting block\n";
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
  std::set<Instruction *> const &controlInstructions
  ){
  auto BBs = LDI.getLoopSummary()->getBasicBlocks();
  auto fn = [&BBs, &instsToPullOut, &controlInstructions](Value *toOrFrom, DataDependenceType ddType) -> bool {
    if (!isa<Instruction>(toOrFrom)) {
      return false;
    }
    auto i = cast<Instruction>(toOrFrom);

    /*
     * Ignore dependencies between instructions we are pulling out and control instructions.
     *   It is okay for an instruction to depend on a control instruction, because control
     *   instructions will be cloned. It is impossible for a control instruction to depend on
     *   a non-control instruction (by definition).
     *   TODO(lukas): Confirm the above
     */
    if (true
        && instsToPullOut.find(i) == instsToPullOut.end()
        && controlInstructions.find(i) == controlInstructions.end()
       ) {
      auto bb = i->getParent();

      /*
       * Only dependencies inside the loop should cause us to abort
       */
      if (std::find(BBs.begin(), BBs.end(), bb) != BBs.end()) {
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
 *   TODO(Lukas): Too conservative, change to assert
 */
bool LoopDistribution::allInstsToPullOutControlDependOnLoopExitingBlock (
  LoopDependenceInfo const &LDI,
  std::set<Instruction *> const &instsToPullOut
  ){

  /*
   * Get a set of the instructions that control-depend on the loop exit
   */
  std::set<Instruction *> controlDependsOnExit{};
  auto loopExitBlocks = LDI.getLoopSummary()->getLoopExitBasicBlocks();
  for (auto loopExitBlock : loopExitBlocks) {

    /*
     * An exit block should have a single predecessor
     */
    auto loopExitingBlock = loopExitBlocks[0]->getSinglePredecessor();
    assert(loopExitingBlock);
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
  }

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
  auto loopSummary = LDI.getLoopSummary();
  auto &cxt = loopSummary->getFunction()->getContext();
  errs() << "LoopDistribution: About to do split of " << *loopSummary->getFunction() << "\n";

  /*
   * Duplicate the basic blocks of the loop and insert clones of all necessary
   *   non-branch instructions in order (instsToPullOut and controlInstructions)
   */
  std::unordered_map<Instruction *, Instruction *> instMap{};
  std::unordered_map<BasicBlock *, BasicBlock *> bbMap{};
  for (auto &BB : loopSummary->getBasicBlocks()) {
    auto cloneBB = BasicBlock::Create(cxt, "", loopSummary->getFunction());
    bbMap[BB] = cloneBB;
    IRBuilder<> builder(cloneBB);
    for (auto &I : *BB) {
      if (isa<BranchInst>(I)) { // TODO(lukas): Should this be all terminators?
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
   * Collect the exiting basic blocks of the original loop. This needs to happen
   *   before we add branches to the new loop or getSinglePredecessor won't work
   */
  std::unordered_map<BasicBlock *, BasicBlock *> exitBlockToExitingBlock{};
  for (auto exitBlock : loopSummary->getLoopExitBasicBlocks()) {
    auto exitingBlock = exitBlock->getSinglePredecessor();
    assert(exitingBlock);
    exitBlockToExitingBlock[exitBlock] = exitingBlock;
  }
  errs() << "LoopDistribution: Finished collecting exit branches\n";

  /*
   * Map the original loop exit blocks to themselves so in the next section the new loop
   *   will have branches to the original exits
   */
  for (auto loopExitBlock : loopSummary->getLoopExitBasicBlocks()) {
    bbMap[loopExitBlock] = loopExitBlock;
  }

  /*
   * Duplicate all branch instructions (with correct successors).
   *   Cloned branches are not added to instMap because they don't produce values
   */
  for (auto &BB : loopSummary->getBasicBlocks()) {
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
   *   will mess up the process of stitching things together by pointing to blocks not in the map.
   *   New exit blocks are added so that we maintain the single predecessor invarient. These new
   *   exit blocks branch to a preheader which then branches to the new loop's header.
   */
  auto newPreHeader = BasicBlock::Create(cxt, "", loopSummary->getFunction());
  auto newLoopHeader = bbMap.at(loopSummary->getHeader());
  BranchInst::Create(newLoopHeader, newPreHeader);
  bbMap[loopSummary->getPreHeader()] = newPreHeader;
  for (auto pair : exitBlockToExitingBlock) {
    auto oldExitBlock = pair.first;
    auto exitingBlock = pair.second;
    assert(isa<BranchInst>(exitingBlock->getTerminator()));
    auto exitBranch = cast<BranchInst>(exitingBlock->getTerminator());
    auto newExitBlock = BasicBlock::Create(cxt, "", loopSummary->getFunction());
    BranchInst::Create(newPreHeader, newExitBlock);
    for (unsigned idx = 0; idx < exitBranch->getNumSuccessors(); idx++) {
      if (exitBranch->getSuccessor(idx) == oldExitBlock) {
        exitBranch->setSuccessor(idx, newExitBlock);
        break;
      }
    }
  }
  errs() << "LoopDistribution: Finished connecting original loop to new loop\n";

  /*
   * Fix data flows for all instructions in the loop
   */
  for (auto &BB : loopSummary->getBasicBlocks()) {
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
       * Fix data flows that are incoming basic blocks in phi nodes.
       */
      if (auto clonePHI = dyn_cast<PHINode>(&cloneI)) {
        for (unsigned idx = 0; idx < clonePHI->getNumIncomingValues(); idx++) {
          auto oldBB = clonePHI->getIncomingBlock(idx);
          auto newBB = bbMap.at(oldBB);
          clonePHI->setIncomingBlock(idx, newBB);
        }
      }
    }
  }
  errs() << "LoopDistribution: Finished fixing instruction dependencies in the new loop\n";


  /*
   * Fix data flows for all instructions in exit blocks (only need to fix phi nodes)
   */
  for (auto loopExitBlock : loopSummary->getLoopExitBasicBlocks()) {
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
   * Remove instructions from the original loop if they are not control instructions.
   *   Also replace all uses of an instruction with its corresponding clone. This is necessary in
   *   the case that an instruction outside of this loop needs to consume the produced value.
   *   It is always correct to do this because we have already confirmed that there are no uses of
   *   this instruction within the original loop, so any other remaning references are about to 
   *   become null.
   *   TODO(lukas): Confirm this
   */
  for (auto inst : instsToPullOut) {
    if (controlInstructions.find(inst) == controlInstructions.end()) {
      auto cloneInst = instMap.at(inst);
      inst->replaceAllUsesWith(cloneInst);
      inst->eraseFromParent();
    }
  }
  errs() << "LoopDistribution: Finished removing instructions from the original loop\n";

  errs() << "LoopDistribution: Success: Finished split of " << *loopSummary->getFunction() << "\n";
  return ;
  }
