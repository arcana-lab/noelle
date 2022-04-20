/*
 * Copyright 2019 - 2021  Lukas Gross, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/Utils.hpp"
#include "noelle/core/LoopDistribution.hpp"

namespace llvm::noelle {
 
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
  auto loopStructure = LDI.getLoopStructure();
  // errs() << "LoopDistribution: Attempting Loop Distribution in "
  //        << loopStructure->getFunction()->getName()
  //        << "\n";

  /*
   * Assert that all instructions in instsToPullOut are actually within the loop
   */
  auto loopBBs = loopStructure->getBasicBlocks();
  for (auto inst : instsToPullOut) {
    auto parent = inst->getParent();
    // errs() << "LoopDistribution: Asked to pull out " << *inst << "\n";
    assert(std::find(loopBBs.begin(), loopBBs.end(), parent) != loopBBs.end());
  }
  std::set<Instruction *> instsToClone{};

  /*
   * Require that all terminators in the loop are branches and collect instructions that
   *   are dependencies of conditional branches
   */
  for (auto BB : loopStructure->getBasicBlocks()) {
    if (auto branch = dyn_cast<BranchInst>(BB->getTerminator())) {
      // errs () << "LoopDistribution: Branch instruction: " <<  *branch << "\n";
      instsToClone.insert(branch);
      this->recursivelyCollectDependencies(branch, instsToClone, LDI);

    } else {
      // errs() << "LoopDistribution: Abort: Non-branch terminator " << *BB->getTerminator() << "\n";
      return false;
    }
  }

  /*
   * Collect all sub-loop instructions and their dependencies. This does not capture sub-sub loops,
   *   but those BBs should still be in the level 2 loops
   */
  std::set<BasicBlock *> subLoopBBs{};
  auto loopStructureNode = LDI.getLoopHierarchyStructures();
  for (auto childLoopStructureNode : loopStructureNode->getChildren()) {
    //errs() << "LoopDistribution: New sub loop\n";
    auto childLoopStructure = childLoopStructureNode->getLoop();
    for (auto &childBB : childLoopStructure->getBasicBlocks()) {
      subLoopBBs.insert(childBB);
      for (auto &childI : *childBB) {
        // errs() << "LoopDistribution: Sub loop instruction: " << childI << "\n";
        instsToClone.insert(&childI);
        this->recursivelyCollectDependencies(&childI, instsToClone, LDI);
      }
    }
  }

  /*
   * Require that no instruction to pull out is in a sub loop. We can relax this requirement
   *   later, but right now we are faithfully reproducing every sub loop in the new loop
   */
  for (auto inst : instsToPullOut) {
    auto parent = inst->getParent();
    if (subLoopBBs.find(parent) != subLoopBBs.end()) {
      // errs() << "LoopDistribution: Abort: Tried to remove sub loop instruction " << *inst << "\n";
      return false;
    }
  }

  /*
   * Require that all instuctions that we will clone do not have side effects
   *   TODO(lukas): This is very, very conservative
   * Require that all instructions to clone do not have memory dependencies
   */
  auto pdg = LDI.getLoopDG();
  for (auto inst : instsToClone) {
    if (inst->mayHaveSideEffects()){
      // errs() << "LoopDistribution: Abort: Unclonable instruction " << *inst << "\n";
      return false;
    }

    auto fn = [&loopBBs](Value *v, DGEdge<Value> *dependence) -> bool {
      if (!isa<Instruction>(v)) {
        return false;
      }

      /*
       * Only memory dependencies inside the loop should interfere
       */
      auto i = cast<Instruction>(v);
      auto bb = i->getParent();
      return std::find(loopBBs.begin(), loopBBs.end(), bb) != loopBBs.end();
    };

    bool containsMemoryDependencyFrom = pdg->iterateOverDependencesFrom(
      inst,
      false, // Control
      true,  // Memory
      false,  // Register
      fn
    );
    bool containsMemoryDependencyTo = pdg->iterateOverDependencesTo(
      inst,
      false, // Control
      true,  // Memory
      false,  // Register
      fn
    );
    if (containsMemoryDependencyFrom || containsMemoryDependencyTo) {
      return false;
    }

    // errs () << "LoopDistribution: Will clone: " <<  *inst << "\n";
  }

  /*
   * Remove instructions we will clone from instsToPullOut
   */
  for (auto inst : instsToClone) {
    if (instsToPullOut.erase(inst)) {
      // errs() << "LoopDistribution: Removed " << *inst << " from instsToPullOut\n";
    }
  }
  if (instsToPullOut.size() == 0) {
    // errs() << "LoopDistribution: Abort: All instructions requested would have to be cloned\n";
    return false;
  }

  /*
   * Require that there are instructions in the loop besides clone instructions and
   *   the instructions we are pulling out. This avoids an infinite loop of splits
   */
  if (this->splitWouldBeTrivial(loopStructure, instsToPullOut, instsToClone)) {
    // errs() << "LoopDistribution: Abort: Request is trivial and could lead to an infinite loop\n";
    return false;
  }

  /*
   * Require that there are no data dependencies between instsToPullOut and the rest of the loop
   */
  if (this->splitWouldRequireForwardingDataDependencies(LDI, instsToPullOut, instsToClone)) {
    // errs() << "LoopDistribution: Abort: Distribution would require forwarding data dependencies\n";
    return false;
  }

  /*
   * Splitting the loop is now safe
   */
  this->doSplit(
    LDI,
    instsToPullOut,
    instsToClone,
    instructionsRemoved,
    instructionsAdded
  );
  return true;
}


/*
 * Add every instruction that is a dependency of inst to the set toPopulate
 */
void LoopDistribution::recursivelyCollectDependencies (
  Instruction * inst,
  std::set<Instruction *> &toPopulate,
  LoopDependenceInfo const &LDI
  ){
  std::vector<Instruction *> queue = {inst};
  auto BBs = LDI.getLoopStructure()->getBasicBlocks();
  auto pdg = LDI.getLoopDG();
  auto fn = [&BBs, &queue, &toPopulate](Value *from, DGEdge<Value> *dep) -> bool {
    if (!isa<Instruction>(from)) {
      return false;
    }
    auto i = cast<Instruction>(from);

    /*
    * Ignore dependencies that are outside of the loop
    */
    auto parent = i->getParent();
    if (BBs.find(parent) == BBs.end()) {
      return false;
    }

    /*
    * Ignore duplicates
    */
    if (toPopulate.find(i) == toPopulate.end()) {
      // errs() << "LoopDistribution: Found dependency: " << *i << "\n";
      toPopulate.insert(i);
      queue.push_back(i);
    }
    return false;
  };
  while (queue.size() != 0) {
    auto i = queue.back();
    queue.pop_back();
    pdg->iterateOverDependencesTo(
      i,
      false, // Control
      true,  // Memory
      true,  // Register
      fn
    );
  }
  return ;
}

/*
 * Checks if the union of instsToPullOut and instsToClone covers every instruction in the loop
 *   that is not a branch (since we will replicate those anyway)
 */
bool LoopDistribution::splitWouldBeTrivial (
  LoopStructure * const loopStructure,
  std::set<Instruction *> const &instsToPullOut,
  std::set<Instruction *> const &instsToClone
  ){
  auto result = true;
  for (auto &BB : loopStructure->getBasicBlocks()) {
    for (auto &I : *BB) {
      if (true
          && instsToPullOut.find(&I) == instsToPullOut.end()
          && instsToClone.find(&I) == instsToClone.end()
          && (!isa<BranchInst>(&I))
          && (Utils::isActualCode(&I))
        ) {
        //errs() << "LoopDistribution: Not trivial because of " << I << "\n";
        result =  false;
        break ;
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
  std::set<Instruction *> const &instsToClone
  ){
  auto BBs = LDI.getLoopStructure()->getBasicBlocks();
  auto fromFn = [&BBs, &instsToPullOut, &instsToClone]
    (Value *from, DGEdge<Value> *dependence) -> bool {
    if (!isa<Instruction>(from)) {
      return false;
    }
    auto i = cast<Instruction>(from);

    /*
     * Ignore dependencies between instructions we are pulling out. It is okay to have a 
     *   to have a from-dependence with a instruction that will be cloned because those
     *   instructions will still be present in the new loop.
     */
    if (true
        && instsToPullOut.find(i) == instsToPullOut.end()
        && instsToClone.find(i) == instsToClone.end()
    ) {
      auto bb = i->getParent();

      /*
       * Only dependencies inside the loop should cause us to abort
       */
      if (std::find(BBs.begin(), BBs.end(), bb) != BBs.end()) {
        // errs() << "LoopDistribution: Instruction "
              //  << *i << " is the source of a data dependency that would need to be forwarded\n";
        return true;
      }
    }
    return false;
  };
  auto toFn = [&BBs, &instsToPullOut](Value *to, DGEdge<Value> *dependence) -> bool {
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
        // errs() << "LoopDistribution: Instruction "
              //  << *i << " consumes a data dependency that would need to be forwarded\n";
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
      // errs() << "LoopDistribution: Problem was dependency from " << *inst << "\n";
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
      // errs() << "LoopDistribution: Problem was dependency to " << *inst << "\n";
      return true;
    }
  }
  return false;
}

void LoopDistribution::doSplit (
  LoopDependenceInfo const &LDI,
  std::set<Instruction *> const &instsToPullOut,
  std::set<Instruction *> const &instsToClone,
  std::set<Instruction *> &instructionsRemoved,
  std::set<Instruction *> &instructionsAdded
  ){
  auto loopStructure = LDI.getLoopStructure();
  auto &cxt = loopStructure->getFunction()->getContext();
  // errs() << "LoopDistribution: About to do split of " << *loopStructure->getFunction() << "\n";

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
          || instsToClone.find(&I) != instsToClone.end()
      ) {
        auto cloneInst = builder.Insert(I.clone());
        instructionsAdded.insert(cloneInst);
        instMap[&I] = cloneInst;
      }
    }
  }
  // errs() << "LoopDistribution: Finished cloning non-branch instructions\n";

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
  // errs() << "LoopDistribution: Finished collecting exit branches\n";

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
  // errs() << "LoopDistribution: Finished stitching together the new loop CFG\n";

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
    bbMap[oldExitBlock] = newExitBlock;
    instructionsAdded.insert(newExitBlockBranch);
    for (unsigned idx = 0; idx < exitBranch->getNumSuccessors(); idx++) {
      if (exitBranch->getSuccessor(idx) == oldExitBlock) {
        exitBranch->setSuccessor(idx, newExitBlock);
        break;
      }
    }
  }
  // errs() << "LoopDistribution: Finished connecting original loop to new loop\n";

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
  // errs() << "LoopDistribution: Finished fixing instruction dependencies in the new loop\n";

  /*
   * Fix data flows for all instructions in exit blocks (only need to fix phi nodes)
   */
  IRBuilder<> newPreHeaderBuilder(newPreHeader->getFirstNonPHI());
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

        /*
         * IF the incoming value is not split:
         *  Define an intermediate PHI in the new preheader.
         *  Wire the old value into this PHI; use this PHI in the new loop's exit PHI
         * OTHERWISE, directly use the split value (which is guaranteed to dominate the new loop's exit)
         */
        auto it = instMap.find(oldInst);
        if (it == instMap.end()) {
          auto intermediatePHI = newPreHeaderBuilder.CreatePHI(PHI->getType(), 0);
          for (auto originalExitBlock : loopStructure->getLoopExitBasicBlocks()) {
            auto incomingValue = originalExitBlock == loopExitBlock ? oldValue : UndefValue::get(oldValue->getType());
            intermediatePHI->addIncoming(incomingValue, bbMap.at(originalExitBlock));
          }

          PHI->setOperand(0, intermediatePHI);
        } else {
          PHI->setOperand(0, it->second);
        }
      }
    }
  }
  // errs() << "LoopDistribution: Finished fixing instruction dependencies in exit blocks\n";

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
       && instsToClone.find(inst) == instsToClone.end()
       && (!isa<BranchInst>(inst))
    ) {
      auto cloneInst = instMap.at(inst);
      inst->replaceAllUsesWith(cloneInst);
      instructionsRemoved.insert(inst);
      inst->eraseFromParent();
    }
  }
  // errs() << "LoopDistribution: Finished removing instructions from the original loop\n";

  errs() << "LoopDistribution: Success: Finished split of " << *loopStructure->getFunction() << "\n";
  return ;
}

}
