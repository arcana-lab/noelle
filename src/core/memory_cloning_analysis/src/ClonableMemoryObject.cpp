/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
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
#include "arcana/noelle/core/ClonableMemoryObject.hpp"
#include "arcana/noelle/core/Lumberjack.hpp"

namespace arcana::noelle {

std::unordered_set<Instruction *> ClonableMemoryObject::
    getInstructionsUsingLocationOutsideLoop(void) const {
  std::unordered_set<Instruction *> instructions;
  for (auto I : this->castsAndGEPs) {
    if (loop->isIncluded(I))
      continue;
    instructions.insert(I);
  }
  for (auto I : this->storingInstructions) {
    if (loop->isIncluded(I))
      continue;
    instructions.insert(I);
  }
  for (auto I : this->loadInstructions) {
    if (loop->isIncluded(I))
      continue;
    instructions.insert(I);
  }
  for (auto I : this->nonStoringInstructions) {
    if (loop->isIncluded(I))
      continue;
    instructions.insert(I);
  }

  return instructions;
}

bool ClonableMemoryObject::mustAliasAMemoryLocationWithinObject(
    Value *ptr) const {

  /*
   * Same value.
   */
  if (ptr == this->allocation) {
    return true;
  }

  /*
   * Aliases.
   */
  for (auto aliasPtr : this->castsAndGEPs) {
    if (aliasPtr == ptr) {
      return true;
    }
  }

  return false;
}

std::unordered_set<Instruction *> ClonableMemoryObject::
    getLoopInstructionsUsingLocation(void) const {
  std::unordered_set<Instruction *> instructions;
  for (auto I : this->castsAndGEPs) {
    if (!loop->isIncluded(I))
      continue;
    instructions.insert(I);
  }
  for (auto I : this->storingInstructions) {
    if (!loop->isIncluded(I))
      continue;
    instructions.insert(I);
  }
  for (auto I : this->loadInstructions) {
    if (!loop->isIncluded(I))
      continue;
    instructions.insert(I);
  }
  for (auto I : this->nonStoringInstructions) {
    if (!loop->isIncluded(I))
      continue;
    instructions.insert(I);
  }
  return instructions;
}

ClonableMemoryObject::ClonableMemoryObject(AllocaInst *allocation,
                                           uint64_t sizeInBits,
                                           LoopStructure *loop,
                                           DominatorSummary &DS,
                                           PDG *ldg)
  : allocation{ allocation },
    sizeInBits{ sizeInBits },
    loop{ loop },
    isClonable{ false },
    isScopeWithinLoop{ false },
    needInitialization{ false },
    log{ NoelleLumberjack, "ClonableMemoryObject" } {

  log.debug() << "Start\n";

  auto s = log.indentedSection();
  s.onExit(LOG_DEBUG, "Exit\n");

  log.debug() << "Object = " << *allocation << "\n";

  /*
   * Check if the current stack object's scope is the loop.
   */
  this->setObjectScope(allocation, loop, DS);

  /*
   * Identify the instructions that access the stack location.
   */
  this->allocatedType = allocation->getAllocatedType();
  if (!this->identifyStoresAndOtherUsers(loop, DS)) {
    log.debug() << "Cannot identify memory accesses to it\n";
    return;
  }

  /*
   * Check if there is a need for cloning the stack object.
   */
  if ((!this->isThereAMemoryDependenceBetweenLoopIterations(
          loop,
          allocation,
          ldg,
          this->storingInstructions))
      && (!this->isThereAMemoryDependenceBetweenLoopIterations(
          loop,
          allocation,
          ldg,
          this->nonStoringInstructions))
      && (!this->isScopeWithinLoop)) {

    /*
     * There is no need to clone the stack object.
     */
    log.debug() << "Not need to clone this\n";
    return;
  }

  /*
   * Check if the stack object has loop-carried RAW memory data dependences.
   * If it doesn't, then each iteration could have its own copy.
   */
  if (this->isThereRAWThroughMemoryBetweenLoopIterations(loop,
                                                         allocation,
                                                         ldg)) {

    /*
     * The stack object is involved in a loop-carried, RAW, memory data
     * dependence. It cannot be safely cloned.
     */
    log.debug()
        << "There are RAW memory data dependences between loop iterations\n";
    return;
  }

  /*
   * The stack object is not involved in any loop-carried RAW memory data
   * dependences.
   *
   * Check if there are RAW memory dependences between code outside the loop
   * and code within the loop that involves the stack object.
   */
  if (this->isScopeWithinLoop) {

    /*
     * The stack object cannot be accessed outside the loop.
     *
     * Therefore, the object is clonable.
     */
    this->isClonable = true;
    log.debug() << "It is clonable\n";
    return;
  }
  if (!this->isThereRAWThroughMemoryFromLoopToOutside(loop, allocation, ldg)) {
    log.debug() << "It is clonable\n";

    /*
     * The stack object is not involved in any memory RAW data dependence from
     * code within the loop to code outside. In other words, values stored
     * into the stack object within the loop are not read outside.
     *
     * Check if values stored within the stack object outside the loop can be
     * read in the loop.
     */
    if (!this->isThereRAWThroughMemoryFromOutsideToLoop(loop,
                                                        allocation,
                                                        ldg)) {

      /*
       * The stack object is not involved in any memory RAW data dependence
       * between code outside and inside the loop.
       *
       * Therefore, the object is clonable.
       */
      this->isClonable = true;
      return;
    }

    /*
     * Values stored in the stack object before executing the loop could be
     * read within the loop. So we need to initialize the cloned object with
     * the original stack object.
     */
    this->needInitialization = true;
    this->isClonable = true;
    log.debug() << "It requires initialization\n";
    return;
  }

  /*
   * Only consider struct and integer types for objects that has scope outside
   * the loop.
   * TODO: Remove this when array/vector types are supported
   */
  if ((!this->isScopeWithinLoop) && (!allocatedType->isStructTy())
      && (!allocatedType->isIntegerTy())) {
    return;
  }

  /*
   * For stack objects that have scope not fully contained within the target
   * loop, we need to check if the stack object is completely initialized for
   * every iteration. In other words, 1) there is no RAW loop-carried data
   * dependences that involve this stack object and 2) there is no RAW from
   * outside the loop to inside it.
   */
  this->identifyInitialStoringInstructions(loop, DS);
  if (!this->isScopeWithinLoop) {
    if ((!this->areOverrideSetsFullyCoveringTheAllocationSpace())
        || (this->isThereRAWThroughMemoryFromLoopToOutside(loop,
                                                           allocation,
                                                           ldg))) {
      return;
    }
  }

  /*
   * The location is clonable.
   */
  this->isClonable = true;
  log.debug() << "It is clonable\n";

  return;
}

void ClonableMemoryObject::setObjectScope(AllocaInst *allocation,
                                          LoopStructure *loop,
                                          DominatorSummary &ds) {

  /*
   * Look for lifetime calls in the loop.
   */
  for (auto inst : loop->getInstructions()) {

    /*
     * Check if the current instruction is a call to lifetime intrinsics.
     */
    auto call = dyn_cast<CallInst>(inst);
    if (call == nullptr) {
      continue;
    }
    if (!call->isLifetimeStartOrEnd()) {
      continue;
    }

    /*
     * The current instruction is a call to lifetime intrinsics.
     *
     * Check if it is about the stack object we care.
     */
    auto objectUsed = call->getArgOperand(1);
    if (auto castInst = dyn_cast<CastInst>(objectUsed)) {
      objectUsed = castInst->getOperand(0);
    }
    if (objectUsed == allocation) {

      /*
       * We found a lifetime call about our stack object.
       */
      this->isScopeWithinLoop = true;
      return;
    }
  }

  return;
}

AllocaInst *ClonableMemoryObject::getAllocation(void) const {
  return this->allocation;
}

bool ClonableMemoryObject::isClonableLocation(void) const {
  return this->isClonable;
}

bool ClonableMemoryObject::isInstructionCastOrGEPOfLocation(
    Instruction *I) const {
  if (castsAndGEPs.find(I) != castsAndGEPs.end())
    return true;
  return false;
}

bool ClonableMemoryObject::isInstructionStoringLocation(Instruction *I) const {
  if (storingInstructions.find(I) != storingInstructions.end())
    return true;
  return false;
}

bool ClonableMemoryObject::isInstructionLoadingLocation(Instruction *I) const {
  if (nonStoringInstructions.find(I) != nonStoringInstructions.end())
    return true;
  if (loadInstructions.find(I) != loadInstructions.end())
    return true;
  return false;
}

bool ClonableMemoryObject::isMemCpyInstrinsicCall(CallInst *call) {
  auto calledFn = call->getCalledFunction();
  if (!calledFn || !calledFn->hasName())
    return false;
  auto name = calledFn->getName();
  std::string nameString = std::string(name.bytes_begin(), name.bytes_end());
  return nameString.find("llvm.memcpy") != std::string::npos;
}

bool ClonableMemoryObject::identifyStoresAndOtherUsers(LoopStructure *loop,
                                                       DominatorSummary &DS) {

  /*
   * Determine all uses of the stack location.
   * Ensure they only exist within the loop provided.
   */
  std::queue<Instruction *> allocationUses{};
  allocationUses.push(this->allocation);
  while (!allocationUses.empty()) {

    /*
     * Fetch the current instruction that uses the stack location.
     */
    auto I = allocationUses.front();
    allocationUses.pop();

    /*
     * Check all users of the current instruction.
     */
    for (auto user : I->users()) {

      /*
       * Find storing and non-storing instructions
       */
      if (auto cast = dyn_cast<CastInst>(user)) {

        /*
         * NOTE: Continue without checking if the cast is in the loop
         * We still check the cast's uses of course
         */
        allocationUses.push(cast);
        this->castsAndGEPs.insert(cast);
        continue;
      }
      if (auto gep = dyn_cast<GetElementPtrInst>(user)) {

        /*
         * NOTE: Continue without checking if the gep is in the loop
         * We still check the GEP's uses of course
         */
        allocationUses.push(gep);
        this->castsAndGEPs.insert(gep);
        continue;
      }
      if (auto store = dyn_cast<StoreInst>(user)) {

        /*
         * As straightforward as it gets
         */
        this->storingInstructions.insert(store);

      } else if (auto load = dyn_cast<LoadInst>(user)) {

        /*
         * This instruction reads from the stack object.
         */
        this->loadInstructions.insert(load);

      } else if (auto call = dyn_cast<CallInst>(user)) {

        /*
         * Ignore lifetime instructions
         * TODO: Make use of them to better understand memory liveness
         */
        if (call->isLifetimeStartOrEnd()) {
          continue;
        }

        /*
         * We consider llvm.memcpy as a storing instruction if the use is the
         * dest (first operand)
         */
        auto isMemCpy = ClonableMemoryObject::isMemCpyInstrinsicCall(call);
        auto isUseTheDestinationOp =
            (call->arg_size() == 4) && (call->getArgOperand(0) == I);
        auto isUseTheSourceOp =
            (call->arg_size() == 4) && (call->getArgOperand(1) == I);
        if (isMemCpy && isUseTheDestinationOp) {
          storingInstructions.insert(call);

        } else if (isMemCpy && isUseTheSourceOp) {
          loadInstructions.insert(call);

        } else {
          this->nonStoringInstructions.insert(call);
        }

      } else if (auto inst = dyn_cast<Instruction>(user)) {

        this->nonStoringInstructions.insert(inst);

      } else {

        /*
         * All users must be instructions
         */
        return false;
      }

      /*
       * All users must be within the loop or dominate the loop
       * TODO: Once clonable memory can characterize if it is live out, remove
       * this check
       */
      auto inst = cast<Instruction>(user);
      if (!loop->isIncluded(inst)) {
        auto block = inst->getParent();
        auto header = loop->getHeader();
        if (!DS.DT.dominates(block, header)) {
          return false;
        }
      }

      /*
       * No InvokeInst can receive the allocation in any form
       */
      if (isa<InvokeInst>(inst)) {
        return false;
      }
    }
  }

  return true;
}

bool ClonableMemoryObject::isThereRAWThroughMemoryBetweenLoopIterations(
    LoopStructure *loop,
    AllocaInst *al,
    PDG *ldg) const {
  if (this->isThereRAWThroughMemoryBetweenLoopIterations(
          loop,
          al,
          ldg,
          this->loadInstructions)) {
    return true;
  }

  return false;
}

bool ClonableMemoryObject::isThereAMemoryDependenceBetweenLoopIterations(
    LoopStructure *loop,
    AllocaInst *al,
    PDG *ldg,
    const std::unordered_set<Instruction *> &insts) const {

  /*
   * Check every instruction that could load from the stack location.
   */
  for (auto inst : insts) {

    /*
     * Check if the inst is within the loop.
     */
    if (!loop->isIncluded(inst)) {

      /*
       * The instruction is not included in the loop.
       * We can skip it.
       */
      continue;
    }

    /*
     * The inst is within the loop.
     *
     * Check if there is a loop-carried memory dependence from @inst to another
     * instruction of the loop.
     */
    auto functor = [loop](Value *fromValue, DGEdge<Value, Value> *d) -> bool {
      /*
       * Check if the source of the dependence is with an instruction.
       */
      auto inst = dyn_cast<Instruction>(fromValue);
      if (inst == nullptr) {
        return false;
      }

      /*
       * The source of the dependence is with an instruction.
       *
       * Check if the source of the dependence is outside the loop.
       */
      if (!loop->isIncluded(inst)) {

        /*
         * The source is within the loop.
         */
        return false;
      }

      /*
       * The source of the dependence is with an instruction that is outside the
       * loop.
       *
       * Check if it is a loop-carried one.
       */
      if (!d->isLoopCarriedDependence()) {
        return false;
      }

      /*
       * We found a loop-carried, RAW, memory dependence to @inst.
       * We can stop the search now.
       */
      return true;
    };

    if (ldg->iterateOverDependencesTo(inst, false, true, false, functor)
        || ldg->iterateOverDependencesFrom(inst, false, true, false, functor)) {

      /*
       * We found a loop-carried, RAW, memory dependence that points to the load
       * instruction @inst.
       */
      return true;
    }
  }

  return false;
}

bool ClonableMemoryObject::isThereRAWThroughMemoryBetweenLoopIterations(
    LoopStructure *loop,
    AllocaInst *al,
    PDG *ldg,
    const std::unordered_set<Instruction *> &insts) const {

  /*
   * Check every instruction that could load from the stack location.
   */
  for (auto inst : insts) {

    /*
     * Check if the inst is within the loop.
     */
    if (!loop->isIncluded(inst)) {

      /*
       * The instruction is not included in the loop.
       * We can skip it.
       */
      continue;
    }

    /*
     * The inst is within the loop.
     *
     * Check if there is a loop-carried memory dependence from @inst to another
     * instruction of the loop.
     */
    auto functor = [loop](Value *fromValue, DGEdge<Value, Value> *d) -> bool {
      /*
       * @d must be a memory dependence.
       */
      auto memDep = cast<MemoryDependence<Value, Value>>(d);

      /*
       * Check if the source of the dependence is with an instruction.
       */
      auto inst = dyn_cast<Instruction>(fromValue);
      if (inst == nullptr) {
        return false;
      }

      /*
       * The source of the dependence is with an instruction.
       *
       * Check if the source of the dependence is outside the loop.
       */
      if (!loop->isIncluded(inst)) {

        /*
         * The source is within the loop.
         */
        return false;
      }

      /*
       * The source of the dependence is with an instruction that is outside the
       * loop.
       *
       * Check if the dependence is a RAW.
       */
      if (!memDep->isRAWDependence()) {
        return false;
      }

      /*
       * We found a memory RAW data dependence to @inst.
       * Check if it is a loop-carried one.
       */
      if (!memDep->isLoopCarriedDependence()) {
        return false;
      }

      /*
       * We found a loop-carried, RAW, memory dependence to @inst.
       * We can stop the search now.
       */
      return true;
    };

    if (ldg->iterateOverDependencesTo(inst, false, true, false, functor)) {

      /*
       * We found a loop-carried, RAW, memory dependence that points to the load
       * instruction @inst.
       */
      return true;
    }
  }

  return false;
}

bool ClonableMemoryObject::isThereRAWThroughMemoryFromOutsideToLoop(
    LoopStructure *loop,
    AllocaInst *al,
    PDG *ldg,
    std::unordered_set<Instruction *> insts) const {

  /*
   * Check every instruction given as input.
   */
  for (auto inst : insts) {

    /*
     * Check if the inst is within the loop.
     */
    if (!loop->isIncluded(inst)) {
      continue;
    }

    /*
     * The inst is within the loop.
     *
     * Check if there is a memory dependence from an instruction outside the
     * loop to this inst.
     */
    auto functor = [loop](Value *fromValue, DGEdge<Value, Value> *d) -> bool {
      assert(fromValue != nullptr);
      assert(d != nullptr);

      /*
       * @d must be a memory dependence.
       */
      auto memDep = cast<MemoryDependence<Value, Value>>(d);

      /*
       * Check if the source of the dependence is with an instruction.
       */
      auto inst = dyn_cast<Instruction>(fromValue);
      if (inst == nullptr) {
        return false;
      }

      /*
       * The source of the dependence is with an instruction.
       *
       * Check if the source of the dependence is outside the loop.
       */
      if (loop->isIncluded(inst)) {

        /*
         * The source is within the loop.
         */
        return false;
      }

      /*
       * The source of the dependence is with an instruction that is outside the
       * loop.
       *
       * Check if the dependence is a RAW.
       */
      if (!memDep->isRAWDependence()) {
        return false;
      }

      /*
       * We found a memory RAW from an instruction outside the loop to an
       * instruction inside the loop.
       *
       * We can stop the iteration.
       */
      return true;
    };

    if (ldg->iterateOverDependencesTo(inst, false, true, false, functor)) {

      /*
       * We found a memory RAW from a store inside the loop to a load after the
       * loop.
       */
      return true;
    }
  }

  return false;
}

bool ClonableMemoryObject::isThereRAWThroughMemoryFromLoopToOutside(
    LoopStructure *loop,
    AllocaInst *al,
    PDG *ldg,
    std::unordered_set<Instruction *> insts) const {

  /*
   * Check every instruction given as input.
   */
  for (auto inst : insts) {

    /*
     * Check if the inst is within the loop.
     */
    if (!loop->isIncluded(inst)) {
      continue;
    }

    /*
     * The inst is within the loop.
     *
     * Check if there is a memory dependence from an instruction outside the
     * loop to this inst.
     */
    auto functor = [loop](Value *fromValue, DGEdge<Value, Value> *d) -> bool {
      assert(fromValue != nullptr);
      assert(d != nullptr);

      /*
       * @d must be a memory dependence.
       */
      auto memDep = cast<MemoryDependence<Value, Value>>(d);

      /*
       * Check if the source of the dependence is with an instruction.
       */
      auto inst = dyn_cast<Instruction>(fromValue);
      if (inst == nullptr) {
        return false;
      }

      /*
       * The source of the dependence is with an instruction.
       *
       * Check if the source of the dependence is outside the loop.
       */
      if (loop->isIncluded(inst)) {

        /*
         * The source is within the loop.
         */
        return false;
      }

      /*
       * The source of the dependence is with an instruction that is outside the
       * loop.
       *
       * Check if the dependence is a RAW.
       */
      if (!memDep->isRAWDependence()) {
        return false;
      }

      /*
       * We found a memory RAW from an instruction outside the loop to an
       * instruction inside the loop.
       *
       * We can stop the iteration.
       */
      return true;
    };

    if (ldg->iterateOverDependencesFrom(inst, false, true, false, functor)) {

      /*
       * We found a memory RAW from a store inside the loop to a load after the
       * loop.
       */
      return true;
    }
  }

  return false;
}

bool ClonableMemoryObject::isThereRAWThroughMemoryFromOutsideToLoop(
    LoopStructure *loop,
    AllocaInst *al,
    PDG *ldg) const {

  /*
   * Check every read of the stack object.
   */
  if (false

      || this->isThereRAWThroughMemoryFromOutsideToLoop(loop,
                                                        al,
                                                        ldg,
                                                        this->loadInstructions)

      || this->isThereRAWThroughMemoryFromOutsideToLoop(
          loop,
          al,
          ldg,
          this->nonStoringInstructions)

  ) {
    return true;
  }

  return false;
}

bool ClonableMemoryObject::isThereRAWThroughMemoryFromLoopToOutside(
    LoopStructure *loop,
    AllocaInst *al,
    PDG *ldg) const {

  /*
   * Check every read of the stack object.
   */
  if (false

      || this->isThereRAWThroughMemoryFromLoopToOutside(
          loop,
          al,
          ldg,
          this->storingInstructions)

      || this->isThereRAWThroughMemoryFromLoopToOutside(
          loop,
          al,
          ldg,
          this->nonStoringInstructions)

  ) {
    return true;
  }

  return false;
}

bool ClonableMemoryObject::identifyInitialStoringInstructions(
    LoopStructure *loop,
    DominatorSummary &DS) {

  /*
   * Group non-storing instructions by sets of dominating basic blocks
   * for which any two sets do not dominate each other
   */
  std::unordered_set<Instruction *> instructionsNeedingCoverage;
  for (auto nonStoringInstruction : this->nonStoringInstructions) {
    instructionsNeedingCoverage.insert(nonStoringInstruction);
  }
  for (auto loadInstruction : this->loadInstructions) {
    instructionsNeedingCoverage.insert(loadInstruction);
  }

  for (auto instToCover : instructionsNeedingCoverage) {

    /*
     * Fetch the basic block of the current instruction.
     */
    auto instBlock = instToCover->getParent();

    // nonStoringInstruction->print(errs() << "Grouping non storing instruction:
    // "); errs() << "\n";

    auto belongsToExistingSet = false;
    for (auto &overrideSet : overrideSets) {
      auto overrideSetDominatingBlock =
          overrideSet->dominatingBlockOfNonStoringInsts;
      if (DS.DT.dominates(overrideSetDominatingBlock, instBlock)) {
        overrideSet->subsequentNonStoringInstructions.insert(instToCover);
        belongsToExistingSet = true;

        // overrideSet->dominatingBlockOfNonStoringInsts->printAsOperand(
        //   errs() << "\tIs dominated by override set: "
        // );

        break;
      }
    }

    /*
     * If the instruction hasn't been added to a set, create a new set
     */
    if (belongsToExistingSet)
      continue;

    // errs() << "\tIs not dominated by current override set\n";
    // nonStoringBlock->printAsOperand(errs() << "\tCreating set: "); errs() <<
    // "\n";

    auto overrideSet = std::make_unique<ClonableMemoryObject::OverrideSet>();
    overrideSet->dominatingBlockOfNonStoringInsts = instBlock;
    overrideSet->subsequentNonStoringInstructions.insert(instToCover);
    overrideSets.insert(std::move(overrideSet));
  }

  /*
   * Find which storing instructions belong to which override sets
   */
  for (auto storingInstruction : storingInstructions) {

    /*
     * Only instructions in the loop can possibly override this memory every
     * iteration
     */
    if (!loop->isIncluded(storingInstruction)) {
      continue;
    }

    auto storingBlock = storingInstruction->getParent();

    for (auto &overrideSet : overrideSets) {
      auto overrideSetDominatingBlock =
          overrideSet->dominatingBlockOfNonStoringInsts;
      if (DS.DT.dominates(storingBlock, overrideSetDominatingBlock)) {
        overrideSet->initialStoringInstructions.insert(storingInstruction);

        // overrideSet->dominatingBlockOfNonStoringInsts->printAsOperand(
        //   errs() << "Belongs to override set: "
        // );
        // storingInstruction->print(errs() << "\t"); errs() << "\n";
      }
    }
  }

  /*
   * Ensure all override sets have at least one storing instruction
   */
  for (auto &overrideSet : overrideSets) {
    if (overrideSet->initialStoringInstructions.size() == 0) {

      // overrideSet->dominatingBlockOfNonStoringInsts->printAsOperand(
      //   errs() << "Override set has no initial storing instructions: "
      // );
      // errs() << "\n";

      return false;
    }
  }

  return true;
}

bool ClonableMemoryObject::areOverrideSetsFullyCoveringTheAllocationSpace(
    void) const {
  if (overrideSets.size() == 0) {
    return false;
  }

  for (auto &overrideSet : overrideSets) {
    if (!this->isOverrideSetFullyCoveringTheAllocationSpace(
            overrideSet.get())) {
      return false;
    }
  }

  return true;
}

bool ClonableMemoryObject::isOverrideSetFullyCoveringTheAllocationSpace(
    ClonableMemoryObject::OverrideSet *overrideSet) const {
  std::unordered_set<int64_t> structElementsStoredTo;
  for (auto storingInstruction : overrideSet->initialStoringInstructions) {
    if (auto store = dyn_cast<StoreInst>(storingInstruction)) {

      /*
       * Fetch the pointer of the memory location modified by @store.
       */
      auto pointerOperand = store->getPointerOperand();

      /*
       * If the pointer is the returned value of an alloca, then @store is
       * initializing the whole memory object.
       */
      if (dyn_cast<AllocaInst>(pointerOperand)) {

        /*
         * The allocation is stored directly to and is completely overriden
         */
        return true;
      }

      if (auto gep = dyn_cast<GetElementPtrInst>(pointerOperand)) {

        // gep->print(errs() << "Examining GEP for coverage: "); errs() << "\n";

        /*
         * Only supporting struct GEP accesses that match the allocation's
         * struct type
         */
        auto sourceElementTy = gep->getSourceElementType();
        if (!sourceElementTy->isStructTy())
          continue;
        if (sourceElementTy != this->allocatedType)
          continue;

        /*
         * Ensure the struct itself is accessed (base idx 0)
         * Track element idx stored to
         */
        auto baseIdxIter = gep->idx_begin();
        auto elementIdxIter = baseIdxIter + 1;
        auto baseIdxValue = dyn_cast<ConstantInt>(baseIdxIter->get());
        auto elementIdxValue = dyn_cast<ConstantInt>(elementIdxIter->get());
        if (!baseIdxValue || !elementIdxValue)
          continue;
        if (baseIdxValue->getValue().getSExtValue() != 0)
          continue;

        auto elementIdx = elementIdxValue->getValue().getSExtValue();
        structElementsStoredTo.insert(elementIdx);

        // errs() << "\tElement: " << elementIdx << "\n";
      }

    } else if (auto call = dyn_cast<CallInst>(storingInstruction)) {
      assert(ClonableMemoryObject::isMemCpyInstrinsicCall(call));

      // call->print(errs() << "Examining llvm.memcpy call: "); errs() << "\n";

      /*
       * Naively require that the whole allocation is written to
       */
      auto bytesStoredValue = call->getOperand(2);
      auto bytesStoredConst = dyn_cast<ConstantInt>(bytesStoredValue);
      if (!bytesStoredConst)
        continue;

      auto bitsStored = bytesStoredConst->getValue().getZExtValue() * 8;
      if (this->sizeInBits == bitsStored) {
        return true;
      }
    }
  }

  if (this->allocation->getAllocatedType()->isStructTy()) {

    // errs() << "Number of elements covered: " << structElementsStoredTo.size()
    // << " versus struct element number: " <<
    // this->allocatedType->getStructNumElements() << "\n";

    if (structElementsStoredTo.size()
        == this->allocatedType->getStructNumElements())
      return true;
  }

  return false;
}

std::unordered_set<Instruction *> ClonableMemoryObject::
    getPointersUsedToAccessObject(void) const {
  return this->castsAndGEPs;
}

bool ClonableMemoryObject::doPrivateCopiesNeedToBeInitialized(void) const {
  return this->needInitialization;
}

uint64_t ClonableMemoryObject::getSizeInBits(void) const {
  return this->sizeInBits;
}

} // namespace arcana::noelle
