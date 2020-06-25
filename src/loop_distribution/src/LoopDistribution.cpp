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
  bool modified = this->splitLoop(LDI, Insts, instructionsRemoved, instructionsAdded);
  return modified;
}


bool LoopDistribution::splitLoop (
  LoopDependenceInfo const &LDI,
  std::set<Instruction *> &instsToPullOut,
  std::set<Instruction *> &instructionsRemoved,
  std::set<Instruction *> &instructionsAdded
  ){
  errs() << "LoopDistribution: Attempting Loop Distribution\n";
  auto loopStructure = LDI.getLoopStructure();
  errs() << "LoopDistribution: The nesting level is " << loopStructure->getNestingLevel() << "\n";
  errs() << "LoopDistribution: Number of child loops is " << loopStructure->getChildren().size() << "\n";

  /*
   * Assert that all instructions in instsToPullOut are actually within the loop
   */
  auto loopBBs = loopStructure->getBasicBlocks();
  for (auto inst : instsToPullOut) {
    auto parent = inst->getParent();
    errs() << "LoopDistribution: Asked to pull out " << *inst << "\n";
    assert(std::find(loopBBs.begin(), loopBBs.end(), parent) != loopBBs.end());
  }

  /*
   * Collect control instructions from the SCCs with loop-carried control dependencies
   */
  auto controlSCCs = LDI.sccdagAttrs.getSCCsWithLoopCarriedControlDependencies();
  std::set<Instruction *> controlInstructions{};
  for (auto controlSCC : controlSCCs) {
    errs() << "LoopDistribution: New Control SCC\n";
    for (auto pair : controlSCC->internalNodePairs()) {
      if (auto controlInst = dyn_cast<Instruction>(pair.second->getT())) {
        errs () << "LoopDistribution: Control instruction from SCC: " <<  *controlInst << "\n";
        controlInstructions.insert(controlInst);
      }
    }
  }

  /*
   * Require that all terminators in the loop are branches and collect control instructions that
   *   are dependencies of conditional branches
   */
  for (auto BB : loopStructure->getBasicBlocks()) {
    if (auto branch = dyn_cast<BranchInst>(BB->getTerminator())) {
      if (branch->isConditional()) {
        if (auto condition = dyn_cast<Instruction>(branch->getCondition())) {
          errs () << "LoopDistribution: Colecting dependencies of  " <<  *condition << "\n";
          controlInstructions.insert(condition);
          this->recursivelyCollectDependencies(condition, controlInstructions);
        }      
      }
    } else {
      errs() << "LoopDistribution: Abort: Non-branch terminator " << *BB->getTerminator() << "\n";
    }
  }

  /*
   * Require that all control instuctions are clonable
   */
  for (auto controlInst : controlInstructions) {
    if (controlInst->mayHaveSideEffects()){
      errs() << "LoopDistribution: Abort: Unclonable instruction " << *controlInst << "\n";
      return false;
    }
    errs () << "LoopDistribution: Control instruction: " <<  *controlInst << "\n";
  }

  /*
   * Remove control instructions from instsToPullOut
   */
  for (auto controlInst : controlInstructions) {
    if (instsToPullOut.erase(controlInst)) {
      errs() << "LoopDistribution: Removed " << *controlInst << " from instsToPullOut\n";
    }
  }
  if (instsToPullOut.size() == 0) {
    errs() << "LoopDistribution: Abort: Every instruction was a control instruction\n";
    return false;
  }

  /*
   * Require that all instructions in sub loops are clonable, and collect them
   */
  std::set<Instruction *> subLoopInstructions{};
  for (auto childLoopSummary : loopStructure->getChildren()) {
    errs() << "LoopDistribution: New sub loop\n";
    for (auto &childBB : childLoopSummary->getBasicBlocks()) {
      for (auto &childI : *childBB) {
        if (childI.mayHaveSideEffects()){
          errs() << "LoopDistribution: Abort: Unclonable sub loop instruction " << childI << "\n";
          return false;
        }
        errs() << "LoopDistribution: Sub loop instruction: " << childI << "\n";
        subLoopInstructions.insert(&childI);
      }
    }
  }
  errs() << "LoopDistribution: Found " << subLoopInstructions.size() << " sub loop instructions\n";

  /*
   * Require that no instruction to pull out is in a sub loop. We can relax this requirement
   *   later, but right now we are faithfully reproducing every sub loop in the new loop
   */
  for (auto inst : instsToPullOut) {
    if (subLoopInstructions.find(inst) != subLoopInstructions.end()) {
      errs() << "LoopDistribution: Abort: Tried to remove sub loop instruction " << *inst << "\n";
      return false;
    }
  }

  /*
   * Require that there are instructions in the loop besides control instructions and
   *   the instructions we are pulling out. This avoids an infinite loop of splits
   */
  if (this->splitWouldBeTrivial(
      loopStructure,
      instsToPullOut,
      controlInstructions,
      subLoopInstructions
      )
    ) {
    errs() << "LoopDistribution: Abort: Request is trivial and could lead to an infinite loop\n";
    return false;
  }

  /*
   * Require that there are no data dependencies between instsToPullOut and the rest of the loop
   */
  if (this->splitWouldRequireForwardingDataDependencies(
        LDI,
        instsToPullOut,
        controlInstructions,
        subLoopInstructions
      )
    ) {
    errs() << "LoopDistribution: Abort: Distribution would require forwarding data dependencies\n";
    return false;
  }

  /*
   * Splitting the loop is now safe
   */
  this->doSplit(
    LDI,
    instsToPullOut,
    controlInstructions,
    subLoopInstructions,
    instructionsRemoved,
    instructionsAdded
  );
  return true;
}


/*
 * Add every instruction that is a dependency of inst to the set toPopulate. We don't need to worry
 *   about aliasing because no cloned instruction is allowed to write to memory
 */
void LoopDistribution::recursivelyCollectDependencies (
  Instruction * inst,
  std::set<Instruction *> &toPopulate
  ){
    std::vector<Instruction *> queue = {inst};
    while (queue.size() != 0) {
      auto i = queue.back();
      queue.pop_back();
      for (unsigned idx = 0; idx < i->getNumOperands(); idx++) {
        if (auto dependency = dyn_cast<Instruction>(i->getOperand(idx))) {
          if (toPopulate.find(dependency) == toPopulate.end()) {
            errs () << "LoopDistribution: Found dependency: " << *dependency << "\n";
            toPopulate.insert(dependency);
            queue.push_back(dependency);
          }
        }
      }
    }
  return ;
}

/*
 * Checks if the union of instsToPullOut and controlInsts covers every instruction in the loop
 *   that is not a branch or part of a sub loop (since we will replicate those anyway)
 */
bool LoopDistribution::splitWouldBeTrivial (
  LoopStructure * const loopStructure,
  std::set<Instruction *> const &instsToPullOut,
  std::set<Instruction *> const &controlInstructions,
  std::set<Instruction *> const &subLoopInstructions
  ){
  bool result = true;
  for (auto &BB : loopStructure->getBasicBlocks()) {
    for (auto &I : *BB) {
      if (true
          && instsToPullOut.find(&I) == instsToPullOut.end()
          && controlInstructions.find(&I) == controlInstructions.end()
          && subLoopInstructions.find(&I) == subLoopInstructions.end()
          && (!isa<BranchInst>(&I))
        ) {
        errs() << "LoopDistribution: Not trivial because of " << I << "\n";
        result =  false;
      }
    }
  }
  return result;
}

/*
 * Checks if any instructions in instsToPullOut are the source or destination of a data dependency
 *   to another instruction in the loop that would break if we split the loop
 */
bool LoopDistribution::splitWouldRequireForwardingDataDependencies (
  LoopDependenceInfo const &LDI,
  std::set<Instruction *> const &instsToPullOut,
  std::set<Instruction *> const &controlInstructions,
  std::set<Instruction *> const &subLoopInstructions
  ){
  auto BBs = LDI.getLoopStructure()->getBasicBlocks();
  auto fromFn = [&BBs, &instsToPullOut, &controlInstructions, &subLoopInstructions]
    (Value *from, DataDependenceType ddType) -> bool {
    if (!isa<Instruction>(from)) {
      return false;
    }
    auto i = cast<Instruction>(from);

    /*
     * Ignore dependencies between instructions we are pulling out. It is okay to have a 
     *   to have a from-dependence with a control instruction or a sub loop instruction
     *   because those instructions will still be present in the new loop.
     */
    if (true
        && instsToPullOut.find(i) == instsToPullOut.end()
        && controlInstructions.find(i) == controlInstructions.end()
        && subLoopInstructions.find(i) == subLoopInstructions.end()
    ) {
      auto bb = i->getParent();

      /*
       * Only dependencies inside the loop should cause us to abort
       */
      if (std::find(BBs.begin(), BBs.end(), bb) != BBs.end()) {
        errs() << "LoopDistribution: Instruction "
               << *i << " is the source of a data dependency that would need to be forwarded\n";
        return true;
      }
    }
    return false;
  };
  auto toFn = [&BBs, &instsToPullOut](Value *to, DataDependenceType ddType) -> bool {
    if (!isa<Instruction>(to)) {
      return false;
    }
    auto i = cast<Instruction>(to);

    /*
     * Ignore dependencies between instructions we are pulling out. We can't have dependencies to
     *   cloned instructions because we would break them when we pull out the instruction
     */
    if (instsToPullOut.find(i) == instsToPullOut.end()) {
      auto bb = i->getParent();

      /*
       * Only dependencies inside the loop should cause us to abort
       */
      if (std::find(BBs.begin(), BBs.end(), bb) != BBs.end()) {
        errs() << "LoopDistribution: Instruction "
               << *i << " consumes a data dependency that would need to be forwarded\n";
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
      toFn
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
      fromFn
    );
    if (isDestinationOfExternalDataDependency) {
      errs() << "LoopDistribution: Problem was dependency to " << *inst << "\n";
      return true;
    }
  }
  return false;
}

void LoopDistribution::doSplit (
  LoopDependenceInfo const &LDI,
  std::set<Instruction *> const &instsToPullOut,
  std::set<Instruction *> const &controlInstructions,
  std::set<Instruction *> const &subLoopInstructions,
  std::set<Instruction *> &instructionsRemoved,
  std::set<Instruction *> &instructionsAdded
  ){
  auto loopStructure = LDI.getLoopStructure();
  auto &cxt = loopStructure->getFunction()->getContext();
  errs() << "LoopDistribution: About to do split of " << *loopStructure->getFunction() << "\n";

  /*
   * Duplicate the basic blocks of the loop and insert clones of all necessary
   *   non-branch instructions in order
   */
  std::unordered_map<Instruction *, Instruction *> instMap{};
  std::unordered_map<BasicBlock *, BasicBlock *> bbMap{};
  for (auto &BB : loopStructure->getBasicBlocks()) {
    auto cloneBB = BasicBlock::Create(cxt, "", loopStructure->getFunction());
    bbMap[BB] = cloneBB;
    IRBuilder<> builder(cloneBB);
    for (auto &I : *BB) {
      if (isa<BranchInst>(I)) {
        continue;
      }
      if (false
          || instsToPullOut.find(&I) != instsToPullOut.end()
          || controlInstructions.find(&I) != controlInstructions.end()
          || subLoopInstructions.find(&I) != subLoopInstructions.end()
      ) {
        auto cloneInst = builder.Insert(I.clone());
        instructionsAdded.insert(cloneInst);
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
  for (auto exitBlock : loopStructure->getLoopExitBasicBlocks()) {
    auto exitingBlock = exitBlock->getSinglePredecessor();
    assert(exitingBlock);
    exitBlockToExitingBlock[exitBlock] = exitingBlock;
  }
  errs() << "LoopDistribution: Finished collecting exit branches\n";

  /*
   * Map the original loop exit blocks to themselves so in the next section the new loop
   *   will have branches to the original exits
   */
  for (auto loopExitBlock : loopStructure->getLoopExitBasicBlocks()) {
    bbMap[loopExitBlock] = loopExitBlock;
  }

  /*
   * Duplicate all branch instructions (with correct successors).
   *   Cloned branches are not added to instMap because they don't produce values
   */
  for (auto &BB : loopStructure->getBasicBlocks()) {
    IRBuilder<> builder(bbMap.at(BB));
    auto terminator = BB->getTerminator();
    auto cloneTerminator = builder.Insert(terminator->clone());
    instructionsAdded.insert(cloneTerminator);
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
  auto newPreHeader = BasicBlock::Create(cxt, "", loopStructure->getFunction());
  auto newLoopHeader = bbMap.at(loopStructure->getHeader());
  auto newPreHeaderBranch = BranchInst::Create(newLoopHeader, newPreHeader);
  instructionsAdded.insert(newPreHeaderBranch);
  bbMap[loopStructure->getPreHeader()] = newPreHeader;
  for (auto pair : exitBlockToExitingBlock) {
    auto oldExitBlock = pair.first;
    auto exitingBlock = pair.second;
    assert(isa<BranchInst>(exitingBlock->getTerminator()));
    auto exitBranch = cast<BranchInst>(exitingBlock->getTerminator());
    auto newExitBlock = BasicBlock::Create(cxt, "", loopStructure->getFunction());
    auto newExitBlockBranch = BranchInst::Create(newPreHeader, newExitBlock);
    instructionsAdded.insert(newExitBlockBranch);
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
  for (auto &BB : loopStructure->getBasicBlocks()) {
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
  for (auto loopExitBlock : loopStructure->getLoopExitBasicBlocks()) {
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
   * Remove instructions from the original loop if they were not cloned and are not branches.
   *   Also replace all uses of an instruction with its corresponding clone. This is necessary in
   *   the case that an instruction outside of this loop needs to consume the produced value.
   *   It is always correct to do this because we have already confirmed that there are no uses of
   *   this instruction within the original loop, so any other remaning references are about to 
   *   become null.
   */
  for (auto inst : instsToPullOut) {
    if (true
       && controlInstructions.find(inst) == controlInstructions.end()
       && subLoopInstructions.find(inst) == subLoopInstructions.end()
       && (!isa<BranchInst>(inst))
    ) {
      auto cloneInst = instMap.at(inst);
      inst->replaceAllUsesWith(cloneInst);
      instructionsRemoved.insert(inst);
      inst->eraseFromParent();
    }
  }
  errs() << "LoopDistribution: Finished removing instructions from the original loop\n";

  errs() << "LoopDistribution: Success: Finished split of " << *loopStructure->getFunction() << "\n";
  return ;
}
