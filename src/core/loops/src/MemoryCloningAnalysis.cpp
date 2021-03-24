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
using namespace llvm::noelle;

MemoryCloningAnalysis::MemoryCloningAnalysis (
    LoopStructure *loop, 
    DominatorSummary &DS,
    PDG *ldg
    ){ 
  assert(loop != nullptr);
  assert(ldg != nullptr);

  /*
   * Collect allocations at the top of the function
   *
   * NOTE: The assumption is that all AllocaInst are stated before any other instructions
   */
  std::unordered_set<AllocaInst *> allocations;
  auto function = loop->getFunction();
  auto& entryBlock = function->getEntryBlock();
  errs() << "XAN: START ANALYSIS\n";
  for (auto &I : entryBlock) {

    /*
     * We only handle stack objects.
     */
    auto alloca = dyn_cast<AllocaInst>(&I);
    if (alloca == nullptr){
      continue ;
    }

    allocations.insert(alloca);
  }

  /*
   * Now we need to check whether we can determine at compile time the size of the stack objects.
   *
   * To do this, we first need to fetch the data layout.
   */
  auto &DL = function->getParent()->getDataLayout();

  /*
   * Check each stack object's size.
   */
  for (auto allocation : allocations) {

    /*
     * Check if we know the size in bits of the stack object.
     */
    auto sizeInBitsOptional = allocation->getAllocationSizeInBits(DL);
    if (!sizeInBitsOptional.hasValue()) {
      continue;
    }

    /*
     * Fetch the size of the stack object.
     */
    auto sizeInBits = sizeInBitsOptional.getValue();

    /*
     * Check if the stack object is clonable.
     */
    auto location = std::make_unique<ClonableMemoryLocation>(allocation, sizeInBits, loop, DS, ldg);
    if (!location->isClonableLocation()) {
      continue;
    }

    /*
     * The stack object is clonable.
     */
    errs() << "XAN: this is clonable " << *allocation << "\n";
    this->clonableMemoryLocations.insert(std::move(location));
  }
  errs() << "XAN: END ANALYSIS\n";

  return ;
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
  for (auto &location : this->clonableMemoryLocations) {
    if (location->getAllocation() == I) {
      return location.get();
    }
    if (location->isInstructionCastOrGEPOfLocation(I)) {
      return location.get();
    }
    if (location->isInstructionLoadingLocation(I)) {
      return location.get();
    }
    if (location->isInstructionStoringLocation(I)) {
      return location.get();
    }
    if (auto callInst = dyn_cast<CallInst>(I)){
      if (callInst->isLifetimeStartOrEnd()){
        auto ptr = callInst->getArgOperand(1);
        auto loc = location.get();
        if (loc->mustAliasAMemoryLocationWithinObject(ptr)){
          return loc;
        }
      }
    }
  }

  return nullptr;
}

std::unordered_set<Instruction *> ClonableMemoryLocation::getInstructionsUsingLocationOutsideLoop (void) const {
  std::unordered_set<Instruction *> instructions;
  for (auto I : this->castsAndGEPs) {
    if (loop->isIncluded(I)) continue;
    instructions.insert(I);
  }
  for (auto I : this->storingInstructions) {
    if (loop->isIncluded(I)) continue;
    instructions.insert(I);
  }
  errs() << "XAN: CLONING: LOADS = " << &this->loadInstructions << "\n";
  for (auto I : this->loadInstructions) {
    errs() << "XAN: CLONING: AAAA\n";
    if (loop->isIncluded(I)) continue;
    errs() << "XAN: CLONING: AAAA2\n";
    instructions.insert(I);
  }
  for (auto I : this->nonStoringInstructions) {
    if (loop->isIncluded(I)) continue;
    instructions.insert(I);
  }

  return instructions;
}
      
bool ClonableMemoryLocation::mustAliasAMemoryLocationWithinObject (Value *ptr) const {

  /*
   * Same value.
   */
  if (ptr == this->allocation){
    return true;
  }

  /*
   * Aliases.
   */
  for (auto aliasPtr : this->castsAndGEPs){
    if (aliasPtr == ptr){
      return true;
    }
  }

  return false;
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
  for (auto I : this->loadInstructions) {
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
  DominatorSummary &DS,
  PDG *ldg
) : allocation{allocation}
    ,sizeInBits{sizeInBits}
    ,loop{loop}
    ,isClonable{false}
    ,isScopeWithinLoop{false}
{

  /*
   * Check if the current stack object's scope is the loop.
   */
  this->setObjectScope(allocation, loop, DS);

  /*
   * Only consider struct and integer types for objects that has scope outside the loop.
   * TODO: Remove this when array/vector types are supported
   */
  this->allocatedType = allocation->getAllocatedType();
  errs() << "XAN: Checking " << *allocation << "\n";
  if (  true
        && (!this->isScopeWithinLoop)
        && (!allocatedType->isStructTy())
        && (!allocatedType->isIntegerTy()) 
     ){
    return;
  }

  /*
   * Identify the instructions that access the stack location.
   */
  if (!this->identifyStoresAndOtherUsers(loop, DS)) {
    return;
  }

  /*
   * For stack objects that have scope not fully contained within the target loop, we need to check if the stack object is completely initialized for every iteration.
   * In other words, 
   * 1) there is no RAW loop-carried data dependences that involve this stack object and
   * 2) there is no RAW from outside the loop to inside it.
   */
  this->identifyInitialStoringInstructions(DS);
  if (!this->isScopeWithinLoop){
    if (  false
          || (!this->areOverrideSetsFullyCoveringTheAllocationSpace())
          || (this->isThereRAWThroughMemoryFromOutsideLoop(loop, allocation, ldg))
      ){
      return;
    }
  }

  /*
   * The location is clonable.
   */
  errs() << "XAN:   It is clonable\n";
  this->isClonable = true;

  return;
}

void ClonableMemoryLocation::setObjectScope (
  AllocaInst *allocation,
  LoopStructure *loop,
  DominatorSummary &ds
  ) {

  /*
   * Look for lifetime calls in the loop.
   */
  for (auto inst : loop->getInstructions()){

    /*
     * Check if the current instruction is a call to lifetime intrinsics.
     */
    auto call = dyn_cast<CallInst>(inst);
    if (call == nullptr){
      continue ;
    }
    if (!call->isLifetimeStartOrEnd()){
      continue;
    }

    /*
     * The current instruction is a call to lifetime intrinsics.
     *
     * Check if it is about the stack object we care.
     */
    auto objectUsed = call->getArgOperand(1);
    if (auto castInst = dyn_cast<CastInst>(objectUsed)){
      objectUsed = castInst->getOperand(0);
    }
    if (objectUsed == allocation){

      /*
       * We found a lifetime call about our stack object.
       */
      this->isScopeWithinLoop = true;
      return ;
    }
  }

  return ;
}

AllocaInst * ClonableMemoryLocation::getAllocation (void) const {
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
  if (loadInstructions.find(I) != loadInstructions.end()) return true;
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
    errs() << "XAN: Identify uses: " << *I << "\n";
    // I->print(errs() << "Traversing user of allocation: "); errs() << "\n";

    /*
     * Check all users of the current instruction.
     */
    for (auto user : I->users()) {

      /*
       * Find storing and non-storing instructions
       */
      errs() << "XAN: Identify uses:    User " << *user << "\n";
      if (auto cast = dyn_cast<CastInst>(user)) {

        /*
         * NOTE: Continue without checking if the cast is in the loop
         * We still check the cast's uses of course
         */
        errs() << "XAN: Identify uses:      Cast\n";
        allocationUses.push(cast);
        this->castsAndGEPs.insert(cast);
        continue;
      }
      if (auto gep = dyn_cast<GetElementPtrInst>(user)) {

        /*
         * NOTE: Continue without checking if the gep is in the loop
         * We still check the GEP's uses of course
         */
        errs() << "XAN: Identify uses:      GEP\n";
        allocationUses.push(gep);
        this->castsAndGEPs.insert(gep);
        continue;
      } 
      if (auto store = dyn_cast<StoreInst>(user)) {

        /*
         * As straightforward as it gets
         */
        this->storingInstructions.insert(store);

      } else if (auto load = dyn_cast<LoadInst>(user)){

        /*
         * This instruction reads from the stack object.
         */
        errs() << "XAN: Identify uses:      LOAD" << &this->loadInstructions << "\n";
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
         * We consider llvm.memcpy as a storing instruction if the use is the dest (first operand) 
         */
        auto isMemCpy = ClonableMemoryLocation::isMemCpyInstrinsicCall(call);
        auto isUseTheDestinationOp = (call->getNumArgOperands() == 4) && (call->getArgOperand(0) == I);
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
        if (!DS.DT.dominates(block, header)) {
          return false;
        }
      }

      /*
       * No InvokeInst can receive the allocation in any form
       */
      if (auto invokeInst = dyn_cast<InvokeInst>(inst)) {
        return false;
      }
    }
  }

  return true;
}

bool ClonableMemoryLocation::isThereRAWThroughMemoryFromOutsideLoop (
  LoopStructure *loop, 
  AllocaInst *al, 
  PDG *ldg, 
  std::unordered_set<Instruction *> insts
  ) const {

  for (auto inst : insts){

    /*
     * Check if the inst is within the loop.
     */
    if (!loop->isIncluded(inst)){
      continue ;
    }

    /*
     * The inst is within the loop.
     *
     * Check if there is a memory dependence from an instruction outside the loop to this inst.
     */
    auto functor = [loop](Value *fromValue, DGEdge<Value> *d) -> bool{

      /*
       * Check if the source of the dependence is with an instruction.
       */
      auto inst = dyn_cast<Instruction>(fromValue);
      if (inst == nullptr){
        return false;
      }

      /*
       * The source of the dependence is with an instruction.
       *
       * Check if the source of the dependence is outside the loop.
       */
      if (loop->isIncluded(inst)){

        /*
         * The source is within the loop.
         */
        return false;
      }

      /*
       * The source of the dependence is with an instruction that is outside the loop.
       *
       * Check if the dependence is a RAW.
       */
      if (!d->isRAWDependence()){
        return false;
      }

      /*
       * We found a memory RAW from an instruction outside the loop to an instruction inside the loop.
       *
       * We can stop the iteration.
       */
      return true;
    };
    if (ldg->iterateOverDependencesTo(inst, false, true, false, functor)){

      /*
       * We found a memory RAW from outside the loop to inside that is related to our stack object.
       */
      return true;
    }
  }

  return false;
}
        
bool ClonableMemoryLocation::isThereRAWThroughMemoryFromOutsideLoop (LoopStructure *loop, AllocaInst *al, PDG *ldg) const {

  /*
   * Check every read of the stack object.
   */
  if (  false
        || this->isThereRAWThroughMemoryFromOutsideLoop(loop, al, ldg, this->loadInstructions)
        || this->isThereRAWThroughMemoryFromOutsideLoop(loop, al, ldg, this->nonStoringInstructions)
     ){
    return true;
  }


  return false;
}

bool ClonableMemoryLocation::identifyInitialStoringInstructions (DominatorSummary &DS) {

  /*
   * Group non-storing instructions by sets of dominating basic blocks
   * for which any two sets do not dominate each other
   */
  for (auto nonStoringInstruction : this->nonStoringInstructions) {

    /*
     * Fetch the basic block of the current instruction.
     */
    auto nonStoringBlock = nonStoringInstruction->getParent();

    // nonStoringInstruction->print(errs() << "Grouping non storing instruction: "); errs() << "\n";

    auto belongsToExistingSet = false;
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
    if (!this->isOverrideSetFullyCoveringTheAllocationSpace(overrideSet.get())) {
      return false;
    }
  }

  return true;
}

bool ClonableMemoryLocation::isOverrideSetFullyCoveringTheAllocationSpace (
  ClonableMemoryLocation::OverrideSet *overrideSet
) const {
  std::unordered_set<int64_t> structElementsStoredTo;
  for (auto storingInstruction : overrideSet->initialStoringInstructions) {
    if (auto store = dyn_cast<StoreInst>(storingInstruction)) {

      /*
       * Fetch the pointer of the memory location modified by @store.
       */
      auto pointerOperand = store->getPointerOperand();

      /*
       * If the pointer is the returned value of an alloca, then @store is initializing the whole memory object.
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
