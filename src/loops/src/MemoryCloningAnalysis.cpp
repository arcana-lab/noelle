/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "MemoryCloningAnalysis.hpp"

using namespace llvm;
using namespace noelle;

MemoryCloningAnalysis::MemoryCloningAnalysis (LoopStructure *loop, DominatorSummary &DS) {

  /*
   * Collect allocations at the top of the function
   * NOTE: The assumption is that all AllocaInst are stated before any other instructions
   */
  std::unordered_set<AllocaInst *> allocations;
  auto function = loop->getHeader()->getParent();
  auto &entryBlock = function->getEntryBlock();
  for (auto &I : entryBlock) {
    if (auto alloca = dyn_cast<AllocaInst>(&I)) {

      /*
       * Only consider struct and integer types
       * TODO: Expand this to array/vector types
       */
      auto allocatedType = alloca->getAllocatedType();
      if (!allocatedType->isStructTy() && !allocatedType->isIntegerTy()) continue;

      allocations.insert(alloca);
    }
  }

  auto &DL = function->getParent()->getDataLayout();
  for (auto allocation : allocations) {

    auto sizeInBitsOptional = allocation->getAllocationSizeInBits(DL);
    if (!sizeInBitsOptional.hasValue()) continue;

    auto sizeInBits = sizeInBitsOptional.getValue();
    auto location = std::make_unique<ClonableMemoryLocation>(allocation, sizeInBits, loop, DS);
    if (!location->isClonableLocation()) continue;

    this->clonableMemoryLocations.insert(std::move(location));

    // allocation->print(errs() << "Found clonable allocation: "); errs() << "\n";

  }
}

std::unordered_set<ClonableMemoryLocation *> MemoryCloningAnalysis::getClonableMemoryLocations (void) const {
  std::unordered_set<ClonableMemoryLocation *> locations{};
  for (auto &location : this->clonableMemoryLocations) {
    locations.insert(location.get());
  }
  return locations;
}

const ClonableMemoryLocation * MemoryCloningAnalysis::getClonableMemoryLocationFor (Instruction *I) const {

  /*
   * TODO: Determine if it is worth mapping from instructions to locations
   */
  for (auto &location : clonableMemoryLocations) {
    if (location->getAllocation() == I) return location.get();
    if (location->isInstructionCastOrGEPOfLocation(I)) return location.get();
    if (location->isInstructionLoadingLocation(I)) return location.get();
    if (location->isInstructionStoringLocation(I)) return location.get();
  }

  return nullptr;
}

std::unordered_set<Instruction *> ClonableMemoryLocation::getLoopInstructionsUsingLocation (void) const {
  std::unordered_set<Instruction *> instructions;
  for (auto I : this->castsAndGEPs) {
    if (!loop->isIncluded(I)) continue;
    instructions.insert(I);
  }
  for (auto I : this->storingInstructions) {
    if (!loop->isIncluded(I)) continue;
    instructions.insert(I);
  }
  for (auto I : this->nonStoringInstructions) {
    if (!loop->isIncluded(I)) continue;
    instructions.insert(I);
  }
  return instructions;
}

ClonableMemoryLocation::ClonableMemoryLocation (
  AllocaInst *allocation,
  uint64_t sizeInBits,
  LoopStructure *loop,
  DominatorSummary &DS
) : allocation{allocation}, sizeInBits{sizeInBits}, loop{loop}, isClonable{false} {

  /*
   * TODO: Remove this when array/vector types are supported
   */
  this->allocatedType = allocation->getAllocatedType();
  // this->allocation->print(errs() << "Allocation: "); errs() << "\n";
  // this->allocatedType->print(errs() << "Allocation type: "); errs() << "\n";
  if (!allocatedType->isStructTy() && !allocatedType->isIntegerTy()) return;

  if (!identifyStoresAndOtherUsers(loop, DS)) return;

  identifyInitialStoringInstructions(DS);

  if (!areOverrideSetsFullyCoveringTheAllocationSpace()) return;

  isClonable = true;
  return;
}

AllocaInst *ClonableMemoryLocation::getAllocation (void) const {
  return this->allocation;
}

bool ClonableMemoryLocation::isClonableLocation (void) const {
  return this->isClonable;
}

bool ClonableMemoryLocation::isInstructionCastOrGEPOfLocation (Instruction *I) const {
  if (castsAndGEPs.find(I) != castsAndGEPs.end()) return true;
  return false;
}

bool ClonableMemoryLocation::isInstructionStoringLocation (Instruction *I) const {
  if (storingInstructions.find(I) != storingInstructions.end()) return true;
  return false;
}

bool ClonableMemoryLocation::isInstructionLoadingLocation (Instruction *I) const {
  if (nonStoringInstructions.find(I) != nonStoringInstructions.end()) return true;
  return false;
}

bool ClonableMemoryLocation::isMemCpyInstrinsicCall (CallInst *call) {
  auto calledFn = call->getCalledFunction();
  if (!calledFn || !calledFn->hasName()) return false;
  auto name = calledFn->getName();
  std::string nameString = std::string(name.bytes_begin(), name.bytes_end());
  return nameString.find("llvm.memcpy") != std::string::npos;
}

bool ClonableMemoryLocation::identifyStoresAndOtherUsers (LoopStructure *loop, DominatorSummary &DS) {

  /*
   * Determine all stores and non-store uses
   * Ensure they only exist within the loop provided
   */
  std::queue<Instruction *> allocationUses{};
  allocationUses.push(allocation);

  while (!allocationUses.empty()) {
    auto I = allocationUses.front();
    allocationUses.pop();

    // I->print(errs() << "Traversing user of allocation: "); errs() << "\n";

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
        castsAndGEPs.insert(cast);
        continue;

      } else if (auto gep = dyn_cast<GetElementPtrInst>(user)) {

        /*
         * NOTE: Continue without checking if the gep is in the loop
         * We still check the GEP's uses of course
         */
        allocationUses.push(gep);
        castsAndGEPs.insert(gep);
        continue;

      } else if (auto store = dyn_cast<StoreInst>(user)) {

        /*
         * As straightforward as it gets
         */
        storingInstructions.insert(store);

      } else if (auto call = dyn_cast<CallInst>(user)) {

        /*
         * Ignore lifetime instructions
         * TODO: Make use of them to better understand memory liveness
         */
        if (call->isLifetimeStartOrEnd()) continue;

        /*
         * We consider llvm.memcpy as a storing instruction if the use is the dest (first operand) 
         */
        bool isMemCpy = ClonableMemoryLocation::isMemCpyInstrinsicCall(call);
        bool isUseTheDestinationOp = call->getNumArgOperands() == 4
          && call->getArgOperand(0) == I;
        if (isMemCpy && isUseTheDestinationOp) {
          storingInstructions.insert(call);
        } else {
          this->nonStoringInstructions.insert(call);
        }

      } else if (auto inst = dyn_cast<Instruction>(user)) {

        this->nonStoringInstructions.insert(inst);

      } else {

        // user->print(errs() << "Unknown user: "); errs() << "\n";

        /*
         * All users must be instructions
         */
        return false;
      }

      /*
       * All users must be within the loop or dominate the loop
       * TODO: Once clonable memory can characterize if it is live out, remove this check
       */
      auto inst = cast<Instruction>(user);
      // if (!loop->isIncluded(inst)) { inst->print(errs() << "Outside loop!: "); errs() << "\n"; }
      if (!loop->isIncluded(inst)) {
        auto block = inst->getParent();
        auto header = loop->getHeader();
        if (!DS.DT.dominates(block, header)) return false;
      }

      /*
       * No InvokeInst can receive the allocation in any form
       */
      if (auto invokeInst = dyn_cast<InvokeInst>(inst)) return false;

    }
  }

  return true;
}

bool ClonableMemoryLocation::identifyInitialStoringInstructions (DominatorSummary &DS) {

  /*
   * Group non-storing instructions by sets of dominating basic blocks
   * for which any two sets do not dominate each other
   */
  for (auto nonStoringInstruction : nonStoringInstructions) {
    auto nonStoringBlock = nonStoringInstruction->getParent();

    // nonStoringInstruction->print(errs() << "Grouping non storing instruction: "); errs() << "\n";

    bool belongsToExistingSet = false;
    for (auto &overrideSet : overrideSets) {
      auto overrideSetDominatingBlock = overrideSet->dominatingBlockOfNonStoringInsts;
      if (DS.DT.dominates(overrideSetDominatingBlock, nonStoringBlock)) {
        overrideSet->subsequentNonStoringInstructions.insert(nonStoringInstruction);
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
    if (belongsToExistingSet) continue;

    // errs() << "\tIs not dominated by current override set\n";
    // nonStoringBlock->printAsOperand(errs() << "\tCreating set: "); errs() << "\n";

    auto overrideSet = std::make_unique<ClonableMemoryLocation::OverrideSet>();
    overrideSet->dominatingBlockOfNonStoringInsts = nonStoringBlock;
    overrideSet->subsequentNonStoringInstructions.insert(nonStoringInstruction);
    overrideSets.insert(std::move(overrideSet));
  }

  /*
   * Find which storing instructions belong to which override sets
   */
  for (auto storingInstruction : storingInstructions) {
    auto storingBlock = storingInstruction->getParent();

    for (auto &overrideSet : overrideSets) {
      auto overrideSetDominatingBlock = overrideSet->dominatingBlockOfNonStoringInsts;
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

bool ClonableMemoryLocation::areOverrideSetsFullyCoveringTheAllocationSpace (void) const {
  for (auto &overrideSet : overrideSets) {
    if (!isOverrideSetFullyCoveringTheAllocationSpace(overrideSet.get())) return false;
  }

  return true;
}

bool ClonableMemoryLocation::isOverrideSetFullyCoveringTheAllocationSpace (
  ClonableMemoryLocation::OverrideSet *overrideSet
) const {

  std::unordered_set<int64_t> structElementsStoredTo;

  for (auto storingInstruction : overrideSet->initialStoringInstructions) {
    if (auto store = dyn_cast<StoreInst>(storingInstruction)) {

      auto pointerOperand = store->getPointerOperand();
      if (auto allocation = dyn_cast<AllocaInst>(pointerOperand)) {

        /*
         * The allocation is stored directly to and is completely overriden
         */
        return true;

      } else if (auto gep = dyn_cast<GetElementPtrInst>(pointerOperand)) {

        // gep->print(errs() << "Examining GEP for coverage: "); errs() << "\n";

        /*
         * Only supporting struct GEP accesses that match the allocation's struct type
         */
        auto sourceElementTy = gep->getSourceElementType();
        if (!sourceElementTy->isStructTy()) continue;
        if (sourceElementTy != this->allocatedType) continue;

        /*
         * Ensure the struct itself is accessed (base idx 0)
         * Track element idx stored to
         */
        auto baseIdxIter = gep->idx_begin();
        auto elementIdxIter = baseIdxIter + 1;
        auto baseIdxValue = dyn_cast<ConstantInt>(baseIdxIter->get());
        auto elementIdxValue = dyn_cast<ConstantInt>(elementIdxIter->get());
        if (!baseIdxValue || !elementIdxValue) continue;
        if (baseIdxValue->getValue().getSExtValue() != 0) continue;

        auto elementIdx = elementIdxValue->getValue().getSExtValue();
        structElementsStoredTo.insert(elementIdx);

        // errs() << "\tElement: " << elementIdx << "\n";

      }

    } else if (auto call = dyn_cast<CallInst>(storingInstruction)) {
      assert(ClonableMemoryLocation::isMemCpyInstrinsicCall(call));

      // call->print(errs() << "Examining llvm.memcpy call: "); errs() << "\n";

      /*
       * Naively require that the whole allocation is written to
       */
      auto bytesStoredValue = call->getOperand(2);
      auto bytesStoredConst = dyn_cast<ConstantInt>(bytesStoredValue);
      if (!bytesStoredConst) continue;

      auto bitsStored = bytesStoredConst->getValue().getSExtValue() * 8;
      if (this->sizeInBits == bitsStored) return true;
    }
  }

  if (this->allocation->getAllocatedType()->isStructTy()) {

    // errs() << "Number of elements covered: " << structElementsStoredTo.size()
      // << " versus struct element number: " << this->allocatedType->getStructNumElements() << "\n";

    if (structElementsStoredTo.size() == this->allocatedType->getStructNumElements()) return true;
  }

  return false;
}