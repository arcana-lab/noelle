/*
 * Copyright 2016 - 2024  Angelo Matni, Simone Campanoni
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
#include "arcana/noelle/core/LoopIterationSpaceAnalysis.hpp"

namespace arcana::noelle {

LoopIterationSpaceAnalysis::LoopIterationSpaceAnalysis(
    LoopTree *loops,
    InductionVariableManager &ivManager,
    ScalarEvolution &SE)
  : loops{ loops },
    ivManager{ ivManager } {

  /*
   * Map IV instructions to SCEVs for quick lookup
   */
  indexIVInstructionSCEVs(SE);
  if (ivInstructionsBySCEV.size() == 0) {
    return;
  }

  /*
   * Derive memory access information for linear indexing
   * Use memory access information to identify non-overlapping memory accesses
   */
  computeMemoryAccessSpace(SE);
  identifyIVForMemoryAccessSubscripts(SE);
  identifyNonOverlappingAccessesBetweenIterationsAcrossOneLoopInvocation(SE);

  return;
}

bool LoopIterationSpaceAnalysis::
    areInstructionsAccessingDisjointMemoryLocationsBetweenIterations(
        Instruction *I,
        Instruction *J) const {

  /*
   * Fetch the memory access spaces of the two instructions.
   */
  if ((!I) || (!J)
      || (this->accessSpaceByInstruction.find(I)
          == this->accessSpaceByInstruction.end())
      || (this->accessSpaceByInstruction.find(J)
          == this->accessSpaceByInstruction.end())) {
    return false;
  }
  auto accessSpaceI = this->accessSpaceByInstruction.at(I);
  auto accessSpaceJ = this->accessSpaceByInstruction.at(J);

  /*
   * Check if the two spaces can overlap.
   */
  auto areDisjoint =
      this->areMemoryAccessSpaceNotOverlappingOrExactlyTheSame(accessSpaceI,
                                                               accessSpaceJ);

  return areDisjoint;
}

bool LoopIterationSpaceAnalysis::
    areMemoryAccessSpaceNotOverlappingOrExactlyTheSame(
        MemoryAccessSpace *accessSpaceI,
        MemoryAccessSpace *accessSpaceJ) const {

  /*
   * Check we were able to determine the boundaries and strides of both memory
   * access spaces.
   */
  if (!accessSpaceI->isAnalyzed) {
    return false;
  }
  if (!accessSpaceJ->isAnalyzed) {
    return false;
  }
  if (this->spacesThatCannotOverlap.find(accessSpaceI)
      == this->spacesThatCannotOverlap.end()) {
    return false;
  }
  if (this->spacesThatCannotOverlap.find(accessSpaceJ)
      == this->spacesThatCannotOverlap.end()) {
    return false;
  }

  /*
   * We were able to determine the boundaries and strides of both memory access
   * spaces.
   *
   * Check if they can overlap.
   */
  auto &notOverlapSetForI = this->spacesThatCannotOverlap.at(accessSpaceI);
  auto &notOverlapSetForJ = this->spacesThatCannotOverlap.at(accessSpaceJ);
  if (notOverlapSetForI.count(accessSpaceJ)) {
    return true;
  }
  if (notOverlapSetForJ.count(accessSpaceI)) {
    return true;
  }

  return false;
}

bool LoopIterationSpaceAnalysis::
    analyzeToCheckIfMemoryAccessSpaceNotOverlappingOrExactlyTheSame(
        MemoryAccessSpace *accessSpaceI,
        MemoryAccessSpace *accessSpaceJ) const {

  // accessSpaceI->memoryAccessor->print(errs() << "Space I accessor: "); errs()
  // << "\n"; accessSpaceJ->memoryAccessor->print(errs() << "Space J accessor:
  // "); errs() << "\n";

  // errs() << "Size: " << nonOverlappingAccessesBetweenIterations.size()  <<
  // "\n"; for (auto access : nonOverlappingAccessesBetweenIterations) {
  //   access->memoryAccessor->print(errs() << "Accessor that doesn't overlap:
  //   "); errs() << "\n";
  // }

  if (this->nonOverlappingAccessesBetweenIterations.find(accessSpaceI)
      == this->nonOverlappingAccessesBetweenIterations.end()) {
    return false;
  }
  // errs() << "Space I is fine\n";
  if (this->nonOverlappingAccessesBetweenIterations.find(accessSpaceJ)
      == this->nonOverlappingAccessesBetweenIterations.end()) {
    return false;
  }
  // errs() << "Space J is fine\n";

  /*
   * @I accesses different memory location at different iterations.
   *
   * @J accesses different memory location at different iterations.
   *
   * Check if the two memory access spaces are guaranteed to be within the same
   * memory object.
   */
  if (accessSpaceI->memoryAccessorBasePointerSCEV
      != accessSpaceJ->memoryAccessorBasePointerSCEV) {
    return false;
  }

  /*
   * @I and @J are guaranteed to:
   * - access different locations at different iterations of the same loop
   * invocation
   * - access the same memory object
   *
   * Now we need to make sure that @I and @J do not overlap.
   * Case 1: @I and @J have the same exact access space.
   */
  if (accessSpaceI == accessSpaceJ) {
    return true;
  }

  /*
   * Case 2: @I and @J are perfectly aligned.
   * In other words, the memory access of @I will never overlap to any memory
   * accessed by @J at a different iteration (and vice versa).
   */
  auto perfectlyAligned =
      this->isMemoryAccessSpaceEquivalentForTopLoopIVSubscript(accessSpaceI,
                                                               accessSpaceJ);

  return perfectlyAligned;
}

bool LoopIterationSpaceAnalysis::
    isMemoryAccessSpaceEquivalentForTopLoopIVSubscript(
        MemoryAccessSpace *space1,
        MemoryAccessSpace *space2) const {
  assert(space1 != nullptr);
  assert(space2 != nullptr);

  if (space1->subscriptIVs.size() == 0) {
    return false;
  }
  if (space1->subscriptIVs.size() != space2->subscriptIVs.size()) {
    return false;
  }
  if (space1->memoryMinusSCEV != space2->memoryMinusSCEV) {
    return false;
  }

  /*space1->memoryAccessor->print(errs() << "Space 1 accessor: "); errs() <<
   "\t"; space2->memoryAccessor->print(errs() << "Space 2 accessor: "); errs()
   << "\n";
   */

  auto getLoopsForIV =
      [&](InductionVariable *iv) -> std::unordered_set<LoopStructure *> {
    auto stepPHIs = iv->getPHIsInvolvedInComputingIVStep();
    std::unordered_set<LoopStructure *> loopsForIV;
    for (std::unordered_set<PHINode *>::iterator it = stepPHIs.begin();
         it != stepPHIs.end();
         it++) {
      loopsForIV.insert(this->loops->getInnermostLoopThatContains(*it));
    }
    return loopsForIV;
  };

  /*
   * For the memory access spaces to be equivalent per each top loop's
   * iteration, without more extensive analysis, we restrict that all dimensions
   * governed by the top loop's IV must be governed by that IV for BOTH spaces,
   * and governed by the same SCEV derived from that IV
   */
  auto rootLoopStructure = this->loops->getLoop();
  for (auto subscriptIdx = 0u; subscriptIdx < space1->subscriptIVs.size();
       ++subscriptIdx) {
    auto iv1 = space1->subscriptIVs[subscriptIdx].second;
    if (iv1 == nullptr) {
      return false;
    }
    auto iv2 = space2->subscriptIVs[subscriptIdx].second;
    if (iv2 == nullptr) {
      return false;
    }

    auto loops1 = getLoopsForIV(iv1);
    auto loops2 = getLoopsForIV(iv2);
    if (loops1 != loops2) {
      for (std::unordered_set<LoopStructure *>::iterator it1 = loops1.begin();
           it1 != loops1.end();
           it1++) {
        for (std::unordered_set<LoopStructure *>::iterator it2 = loops2.begin();
             it2 != loops2.end();
             it2++) {
          if (rootLoopStructure == *it1 || rootLoopStructure == *it2)
            return false;
        }
      }
    }

    auto scev1 = space1->subscripts[subscriptIdx];
    auto scev2 = space2->subscripts[subscriptIdx];
    for (std::unordered_set<LoopStructure *>::iterator it = loops1.begin();
         it != loops1.end();
         it++) {
      if (rootLoopStructure == *it && scev1 != scev2)
        return false;
    }
  }

  return true;
}

void LoopIterationSpaceAnalysis::indexIVInstructionSCEVs(ScalarEvolution &SE) {
  for (auto loop : this->loops->getLoops()) {
    for (auto iv : ivManager.getInductionVariables(*loop)) {
      for (auto inst : iv->getAllInstructions()) {
        if (!SE.isSCEVable(inst->getType()))
          continue;
        auto scev = SE.getSCEV(inst);

        // scev->getType()->print(errs() << "IV instruction SCEV: ");
        // scev->print(errs() << " ");
        // inst->print(errs() << "\n\tIV I: "); errs() << "\n";

        if (ivInstructionsBySCEV.find(scev) == ivInstructionsBySCEV.end()) {
          ivInstructionsBySCEV.insert(
              std::make_pair(scev, std::unordered_set<Instruction *>{ inst }));
        } else {
          ivInstructionsBySCEV.at(scev).insert(inst);
        }
        ivsByInstruction.insert(std::make_pair(inst, iv));
      }

      for (auto inst : iv->getDerivedSCEVInstructions()) {
        if (!SE.isSCEVable(inst->getType()))
          continue;
        auto scev = SE.getSCEV(inst);

        // scev->getType()->print(errs() << "IV derived instruction SCEV: ");
        // scev->print(errs() << " ");
        // inst->print(errs() << "\n\tIV derived I: "); errs() << "\n";

        if (derivedInstructionsFromIVsBySCEV.find(scev)
            == derivedInstructionsFromIVsBySCEV.end()) {
          derivedInstructionsFromIVsBySCEV.insert(
              std::make_pair(scev, std::unordered_set<Instruction *>{ inst }));
        } else {
          derivedInstructionsFromIVsBySCEV.at(scev).insert(inst);
        }
        ivsByInstruction.insert(std::make_pair(inst, iv));
      }
    }
  }

  return;
}

void LoopIterationSpaceAnalysis::computeMemoryAccessSpace(ScalarEvolution &SE) {
  std::unordered_set<Instruction *> memoryAccessors{};

  /*
   * Fetch the target loop.
   */
  assert(this->loops != nullptr);
  auto targetLoop = this->loops->getLoop();

  for (auto B : targetLoop->getBasicBlocks()) {
    for (auto &I : *B) {

      /*
       * Fetch the memory address of the location that is referenced by the
       * current instruction.
       */
      Value *memoryAccessorValue = nullptr;
      if (auto store = dyn_cast<StoreInst>(&I)) {
        memoryAccessorValue = store->getPointerOperand();
      } else if (auto load = dyn_cast<LoadInst>(&I)) {
        memoryAccessorValue = load->getPointerOperand();
      } else if (auto gep = dyn_cast<GetElementPtrInst>(&I)) {
        memoryAccessorValue = gep;
      } else {
        continue;
      }
      assert(memoryAccessorValue != nullptr);

      /*
       * Check if the memory address is computed within the current function.
       */
      if (auto memoryAccessor = dyn_cast<Instruction>(memoryAccessorValue)) {
        memoryAccessors.insert(memoryAccessor);
      }
    }
  }

  for (auto memoryAccessor : memoryAccessors) {
    if (!SE.isSCEVable(memoryAccessor->getType())) {
      continue;
    }

    /*
     * Construct memory space object to track this accessor
     */
    auto element = this->accessSpaces.insert(std::move(
        std::make_unique<LoopIterationSpaceAnalysis::MemoryAccessSpace>(
            memoryAccessor)));
    auto memAccessSpace = (*element.first).get();
    this->accessSpaceByInstruction.insert(
        std::make_pair(memoryAccessor, memAccessSpace));
    memAccessSpace->memoryAccessorSCEV =
        SE.getSCEV(memAccessSpace->memoryAccessor);

    /*
     * Catalog accesses that pertain to this memory space
     */
    for (auto user : memoryAccessor->users()) {
      if (isa<StoreInst>(user) || isa<LoadInst>(user)
          || isa<GetElementPtrInst>(user)) {
        auto accessor = cast<Instruction>(user);
        this->accessSpaceByInstruction.insert(
            std::make_pair(accessor, memAccessSpace));
      }
    }

    /*
     * Determine the accessed type
     */
    Type *accessedType = nullptr;
    for (auto accesses : this->accessSpaceByInstruction) {
      auto accessor = accesses.first;
      if (auto store = dyn_cast<StoreInst>(accessor)) {
        accessedType = store->getValueOperand()->getType();
      } else if (auto load = dyn_cast<LoadInst>(accessor)) {
        accessedType = load->getType();
      } else if (auto gep = dyn_cast<GetElementPtrInst>(accessor)) {
        accessedType = gep->getType();
      } else
        continue;
      break;
    }
    if (!accessedType) {
      continue;
    }

    /*
     * De-linearize step 0: get element size
     */
    auto ptrToAccessedType = PointerType::getUnqual(accessedType);
    auto efType = SE.getEffectiveSCEVType(ptrToAccessedType);
    memAccessSpace->elementSize = SE.getSizeOfExpr(efType, accessedType);

    // memAccessSpace->memoryAccessor->print(errs() << "Accessor: "); errs() <<
    // "\n"; memAccessSpace->memoryAccessorSCEV->print(errs() << "Accessor SCEV:
    // "); errs() << "\n";

    if (!memAccessSpace->elementSize) {
      continue;
    }

    /*
     * De-linearize: collect parametric SCEV terms, dimension sizes, and
     * computed access SCEVs per dimension
     */
    memAccessSpace->memoryAccessorBasePointerSCEV = dyn_cast<SCEVUnknown>(
        SE.getPointerBase(memAccessSpace->memoryAccessorSCEV));
    if (!memAccessSpace->memoryAccessorBasePointerSCEV) {
      continue;
    }

    auto basePointer = memAccessSpace->memoryAccessorBasePointerSCEV;

    auto accessFunction =
        SE.getMinusSCEV(memAccessSpace->memoryAccessorSCEV, basePointer);
    memAccessSpace->memoryMinusSCEV = accessFunction;
    ScalarEvolutionDelinearization::delinearize(SE,
                                                accessFunction,
                                                memAccessSpace->subscripts,
                                                memAccessSpace->sizes,
                                                memAccessSpace->elementSize);

    if (memAccessSpace->subscripts.size() == 0) {
      if (auto gep =
              dyn_cast<GetElementPtrInst>(memAccessSpace->memoryAccessor)) {
        SmallVector<int, 4> sizes;
        ScalarEvolutionDelinearization::getIndexExpressionsFromGEP(
            SE,
            gep,
            memAccessSpace->subscripts,
            sizes);
        for (auto size : sizes) {
          memAccessSpace->sizes.push_back(
              SE.getConstant(accessFunction->getType(), size));
        }

        /*
         * TODO: Determine exactly under what conditions size isn't returned
         * from this API For now, at least include the element size if it is a
         * one dimensional access
         */
        if (sizes.empty()) {
          memAccessSpace->sizes.push_back(memAccessSpace->elementSize);
        }
      }
    }

    /*
     * All dimension's subscripts must be single add-recursive expressions
     * i.e. delinearization must have been factored out all dimensions
     */
    auto isFullyDelinearized = true;
    for (auto i = 0u; i < memAccessSpace->subscripts.size(); ++i) {
      auto subscriptI = memAccessSpace->subscripts[i];
      if (auto addRecSubscript = dyn_cast<SCEVAddRecExpr>(subscriptI)) {
        if (isa<SCEVAddRecExpr>(addRecSubscript->getStart())
            || isa<SCEVAddRecExpr>(addRecSubscript->getStepRecurrence(SE))) {
          isFullyDelinearized = false;
          break;
        }
      }
    }
    if (isFullyDelinearized) {
      memAccessSpace->isAnalyzed = true;

    } else {
      memAccessSpace->subscripts.clear();
      memAccessSpace->sizes.clear();
    }

    // basePointer->print(errs() << "Base pointer: "); errs() << "\n";
    // accessFunction->print(errs() << "Access function: "); errs() << "\n";
    // for (auto i = 0; i < memAccessSpace->subscripts.size(); ++i) {
    //   auto subscript = memAccessSpace->subscripts[i];
    //   subscript->getType()->print(errs() << "Subscript " << i << ": ");
    //   subscript->print(errs() << " " ); errs() << "\n";
    // }
    // for (auto i = 0; i < memAccessSpace->sizes.size(); ++i) {
    //   auto size = memAccessSpace->sizes[i];
    //   size->getType()->print(errs() << "Size " << i << ": ");
    //   size->print(errs() << " " ); errs() << "\n";
    // }
    // errs() << "---------\n";
  }

  return;
}

// TODO: Make this examine the IV and the subscript SCEV separately
// The IV should be used in conjunction with the subscript SCEV to determine
// boundedness The subscript SCEV alone should be used to determine whether it
// is one to one
void LoopIterationSpaceAnalysis::
    identifyNonOverlappingAccessesBetweenIterationsAcrossOneLoopInvocation(
        ScalarEvolution &SE) {

  for (auto &memAccessSpace : this->accessSpaces) {
    if (memAccessSpace->subscriptIVs.size() == 0)
      continue;
    if (memAccessSpace->subscriptIVs.size() != memAccessSpace->sizes.size())
      continue;

    // memAccessSpace->memoryAccessor->print(errs() << "Checking accessor for
    // overlapping: "); errs() << "\n";

    /*
     * Check if the SCEV has the nuw and nsw attributes.
     * In that case, overflow is not possible.
     */
    auto space = memAccessSpace.get();
    auto isOverFlowPossible = true;
    auto scevExpression = dyn_cast<SCEVNAryExpr>(space->memoryAccessorSCEV);
    if (scevExpression != nullptr) {

      /*
       * Find the single operand that computes the offset of an object.
       */
      auto isBound = true;
      auto foundOffsetExpression = false;
      for (auto operandID = 0u; operandID < scevExpression->getNumOperands();
           operandID++) {
        auto operand = scevExpression->getOperand(operandID);

        /*
         * Check if the current operand is a constant (so an object pointer)
         */
        if (isa<SCEVConstant>(operand)) {
          continue;
        }
        if (auto scevUnknown = dyn_cast<SCEVUnknown>(operand)) {
          auto v = scevUnknown->getValue();
          if (isa<Argument>(v)) {
            continue;
          }
        }

        /*
         * Check if we found an operand that computes the offset with an
         * arithmentic expression.
         */
        auto offsetExpression = dyn_cast<SCEVNAryExpr>(operand);
        if (offsetExpression == nullptr) {

          /*
           * We do not know this operand, so we must be conservative: the
           * computation of the address is not bounded.
           */
          isBound = false;
          break;
        }

        /*
         * We found an operand that computes the offset with an arithmentic
         * expression. Check this is the only one.
         */
        if (foundOffsetExpression) {
          isBound = false;
          break;
        }
        foundOffsetExpression = true;
        if (offsetExpression->hasNoSelfWrap()) {

          /*
           * The offset expression cannot wrap.
           */
          continue;
        }
        if (offsetExpression->hasNoSignedWrap()
            && offsetExpression->hasNoUnsignedWrap()) {

          /*
           * The offset expression cannot wrap.
           */
          continue;
        }

        /*
         * We found an arithmetic expression that computes the offset of a
         * memory location and it can wrap. So this is not bound.
         */
        isBound = false;
        break;
      }
      if (isBound && foundOffsetExpression) {
        isOverFlowPossible = false;
      }
    }
    if (isOverFlowPossible) {

      /*
       * Each inner dimension's accesses must be bounded not to spill over into
       * another dimension
       */
      if (!isInnerDimensionSubscriptsBounded(SE, space))
        continue;

      // errs() << "\tAccessor has bounded inner dimension accesses\n";

      /*
       * At least one dimension's subscript's IV must evolve in the top-most
       * loop All dimension's subscripts must be governed by an IV and be
       * bounded by the dimension's size
       */
      auto atLeastOneTopLevelNonOverlappingIV = false;
      auto rootLoopStructure = this->loops->getLoop();
      for (auto idx = 0u; idx < memAccessSpace->subscriptIVs.size(); ++idx) {
        auto instIVPair = memAccessSpace->subscriptIVs[idx];
        auto inst = instIVPair.first;
        auto iv = instIVPair.second;
        if (!iv)
          continue;

        // inst->print(errs() << "Checking if I is non-overlapping root
        // subscript:
        // "); errs() << "\n";

        auto loopEntryPHI = iv->getLoopEntryPHI();
        auto loopStructure =
            this->loops->getInnermostLoopThatContains(loopEntryPHI);
        bool isRootLoopIV = (rootLoopStructure == loopStructure);
        if (!isRootLoopIV)
          continue;

        // SE.getSCEV(inst)->print(errs() << "\tSCEV for root I: "); errs() <<
        // "\n";

        /*
         * If the IV is the root loop's IV, the accesses must be one-to-one
         */
        bool isIV = iv->isIVInstruction(inst);
        bool isDerivedFromIV = iv->isDerivedFromIVInstructions(inst);
        assert(
            (isIV || isDerivedFromIV)
            && "Subscript associated to IV has invalid associated instruction");

        bool isOneToOne;
        if (isIV) {
          // HACK: Currently relies on de-linearization to determine if wrap is
          // likely
          // TODO: Also check loop governing IV attribution for signed guard
          bool isWrapping = false;
          isOneToOne = !isWrapping;
        } else {
          isOneToOne = isOneToOneFunctionOnIV(rootLoopStructure, iv, inst);
        }

        // errs() << "\t Is one to one? : " << isOneToOne << "\n";

        if (!isOneToOne)
          continue;
        atLeastOneTopLevelNonOverlappingIV |= isRootLoopIV;
      }
      if (!atLeastOneTopLevelNonOverlappingIV)
        continue;
    }

    /*
     * The space is not overlapping between iterations
     */
    this->nonOverlappingAccessesBetweenIterations.insert(memAccessSpace.get());
  }

  // errs() << "Non overlapping size: " <<
  // nonOverlappingAccessesBetweenIterations.size() << "\n"; for (auto space :
  // nonOverlappingAccessesBetweenIterations) {
  //   space->memoryAccessor->print(errs() << "Non overlapping space: "); errs()
  //   << "\n";
  // }

  for (auto accessSpacePair0 : this->accessSpaceByInstruction) {
    auto accessSpace0 = accessSpacePair0.second;
    for (auto accessSpacePair1 : this->accessSpaceByInstruction) {
      auto accessSpace1 = accessSpacePair1.second;
      if (accessSpace0 == accessSpace1) {
        continue;
      }
      auto areDisjoint =
          this->analyzeToCheckIfMemoryAccessSpaceNotOverlappingOrExactlyTheSame(
              accessSpace0,
              accessSpace1);
      auto &notOverlappingSet = this->spacesThatCannotOverlap[accessSpace0];
      if (areDisjoint) {
        notOverlappingSet.insert(accessSpace1);
      }
    }
  }

  return;
}

void LoopIterationSpaceAnalysis::identifyIVForMemoryAccessSubscripts(
    ScalarEvolution &SE) {

  auto findCorrespondingIVForSubscript = [&](const SCEV *subscriptSCEV)
      -> std::pair<Instruction *, InductionVariable *> {
    /*
     * Constant subscripts are not linked to IVs
     */
    auto emptyPair = std::make_pair(nullptr, nullptr);
    if (isa<SCEVConstant>(subscriptSCEV))
      return emptyPair;

    auto scevsMatch = [](const SCEV *scev1, const SCEV *scev2) -> bool {
      if (scev1 == scev2)
        return true;
      auto scevConstant1 = dyn_cast<SCEVConstant>(scev1);
      auto scevConstant2 = dyn_cast<SCEVConstant>(scev2);
      if (!scevConstant1 || !scevConstant2)
        return false;
      return scevConstant1->getValue()->getSExtValue()
             == scevConstant2->getValue()->getSExtValue();
    };

    /*
     * Identify an InductionVariable this SCEV belongs to by looking only within
     * the SCEV's loop
     *
     * NOTE: This could result in one of several IVs being found if there exist
     * more than one evolving in lock-step
     */
    auto findInstructionInLoopForSCEV =
        [&SE, &scevsMatch](
            std::unordered_map<const SCEV *, std::unordered_set<Instruction *>>
                &scevToInstMap,
            const SCEV *subscriptSCEV) -> Instruction * {
      if (scevToInstMap.find(subscriptSCEV) != scevToInstMap.end()) {
        auto matchingIVInstruction = *scevToInstMap.at(subscriptSCEV).begin();
        return matchingIVInstruction;
      }

      if (auto addRecSubscriptSCEV = dyn_cast<SCEVAddRecExpr>(subscriptSCEV)) {
        auto loopHeader = addRecSubscriptSCEV->getLoop()->getHeader();
        for (auto scevInstPair : scevToInstMap) {
          auto otherSCEV = scevInstPair.first;
          if (auto otherAddRecSCEV = dyn_cast<SCEVAddRecExpr>(otherSCEV)) {
            if (otherAddRecSCEV->getLoop()->getHeader() != loopHeader)
              continue;
            if (!scevsMatch(addRecSubscriptSCEV->getStart(),
                            otherAddRecSCEV->getStart()))
              continue;
            if (!scevsMatch(addRecSubscriptSCEV->getStepRecurrence(SE),
                            otherAddRecSCEV->getStepRecurrence(SE)))
              continue;
          }

          return *scevInstPair.second.begin();
        }
      }

      return nullptr;
    };

    // subscriptSCEV->print(errs() << "Searching for instruction matching
    // subscript SCEV: "); errs() << "\n";
    auto ivInst =
        findInstructionInLoopForSCEV(this->ivInstructionsBySCEV, subscriptSCEV);
    if (ivInst) {
      // ivInst->print(errs() << "IV INST: "); errs() << "\n";
      auto iv = ivsByInstruction.at(ivInst);
      // iv->getLoopEntryPHI()->print(errs() << "IV LOOP ENTRY PHI: "); errs()
      // << "\n";
      return std::make_pair(ivInst, iv);
    }

    auto derivedInst =
        findInstructionInLoopForSCEV(this->derivedInstructionsFromIVsBySCEV,
                                     subscriptSCEV);
    if (derivedInst) {
      // derivedInst->print(errs() << "DERIVED IV INST: "); errs() << "\n";
      auto derivingIV = ivsByInstruction.at(derivedInst);
      // derivingIV->getLoopEntryPHI()->print(errs() << "DERIVING IV LOOP ENTRY
      // PHI: "); errs() << "\n";
      return std::make_pair(derivedInst, derivingIV);
    }

    return emptyPair;
  };

  for (auto &memAccessSpace : this->accessSpaces) {
    auto idx = 0;
    for (auto subscriptSCEV : memAccessSpace->subscripts) {
      if (isa<SCEVConstant>(subscriptSCEV)) {
        memAccessSpace->subscripts[idx] = memAccessSpace->memoryAccessorSCEV;
        subscriptSCEV = memAccessSpace->memoryAccessorSCEV;
      }
      auto ivOrNullptr = findCorrespondingIVForSubscript(subscriptSCEV);
      memAccessSpace->subscriptIVs.push_back(ivOrNullptr);
      idx++;
    }
    if (auto phi = dyn_cast<PHINode>(memAccessSpace->memoryAccessor)) {
      if (idx == 0 && phi->getNumIncomingValues() == 1) {
        auto ivOrNullptr =
            findCorrespondingIVForSubscript(memAccessSpace->memoryAccessorSCEV);

        memAccessSpace->subscriptIVs.push_back(ivOrNullptr);
        assert(memAccessSpace->elementSize != nullptr
               && "elementSize is nullptr!\n");

        memAccessSpace->sizes.push_back(memAccessSpace->elementSize);
        const SCEV *Expr = SE.getSCEV(phi->getIncomingValue(0));
        assert(Expr != nullptr && "Expr is nullptr!\n");

        memAccessSpace->subscripts.push_back(Expr);
      }
    }
  }

  return;
}

LoopIterationSpaceAnalysis::MemoryAccessSpace::MemoryAccessSpace(
    Instruction *memoryAccessor)
  : memoryAccessor{ memoryAccessor },
    memoryAccessorSCEV{ nullptr },
    memoryAccessorBasePointerSCEV{ nullptr },
    memoryMinusSCEV{ nullptr },
    isAnalyzed{ false },
    elementSize{ nullptr } {
  return;
}

LoopIterationSpaceAnalysis::~LoopIterationSpaceAnalysis() {
  accessSpaces.clear();
}

bool LoopIterationSpaceAnalysis::isOneToOneFunctionOnIV(
    LoopStructure *loopStructure,
    InductionVariable *IV,
    Instruction *derivedInstruction) {

  std::queue<Instruction *> derivingInsts;
  std::unordered_set<Instruction *> visited;
  derivingInsts.push(derivedInstruction);

  /*
   * Traverse all deriving operations from the derived instruction to
   * instructions of the IV
   */
  while (!derivingInsts.empty()) {
    auto inst = derivingInsts.front();
    derivingInsts.pop();

    if (IV->isIVInstruction(inst))
      continue;

    // inst->print(errs() << "Checking if derived inst is a one-to-one
    // operation: "); errs() << "\n";

    /*
     * TODO: Make this far less naive
     * FIXME: Ensure the IV is affine and that no multiplication of the IV to
     * itself occurs
     * FIXME: Ensure that no multiplication by 0 occurs
     */
    auto op = inst->getOpcode();
    bool isOneToOne = (op == Instruction::Add || op == Instruction::Sub
                       || op == Instruction::Mul || inst->isCast());
    if (!isOneToOne)
      return false;

    for (auto &use : inst->operands()) {
      auto usedValue = use.get();

      /*
       * Ignore loop externals and constants as they are computed outside the
       * loop If the user can't be interpreted as an instruction, we do not know
       * how to handle it
       */
      if (isa<ConstantInt>(usedValue))
        continue;
      auto usedInst = dyn_cast<Instruction>(usedValue);
      if (!usedInst)
        return false;
      if (!loopStructure->isIncluded(usedInst))
        continue;

      if (visited.find(usedInst) != visited.end())
        continue;
      visited.insert(usedInst);
      derivingInsts.push(usedInst);
    }
  }

  return true;
}

bool LoopIterationSpaceAnalysis::isInnerDimensionSubscriptsBounded(
    ScalarEvolution &SE,
    MemoryAccessSpace *space) {

  // errs() << "Num subscripts: " << space->subscriptIVs.size() << "\n";
  // errs() << "Num dimensions: " << space->sizes.size() << "\n";
  // for (auto i = 1; i < space->sizes.size(); ++i) {
  //   auto sizeSCEV = space->sizes[i - 1];
  //   auto instIVPair = space->subscriptIVs[i];
  //   auto inst = instIVPair.first;
  //   if (!inst) {
  //     // errs() << "No inst: " << i << "\n";
  //     continue;
  //   }
  //   auto subscriptSCEV = SE.getSCEV(inst);
  //   sizeSCEV->print(errs() << "SIZE "); errs() << "\n";
  //   subscriptSCEV->print(errs() << "Subscript " << i << ": "); errs() <<
  //   "\n"; inst->print(errs() << "\tInst: "); errs() << "\n";
  // }
  if (space->subscriptIVs.size() == 0
      || space->subscriptIVs.size() != space->sizes.size()) {
    return false;
  }

  auto scevsMatch = [](const SCEV *scev1, const SCEV *scev2) -> bool {
    if (scev1 == scev2)
      return true;
    auto scevConstant1 = dyn_cast<SCEVConstant>(scev1);
    auto scevConstant2 = dyn_cast<SCEVConstant>(scev2);
    if (!scevConstant1 || !scevConstant2) {
      return false;
    }
    return scevConstant1->getValue()->getSExtValue()
           == scevConstant2->getValue()->getSExtValue();
  };

  /*
   * All accesses except for the outer-most dimension must be checked for
   * bounded-ness We assume program correctness for the outer-most dimension, as
   * the base program containing memory corruption is out of our hands
   */
  for (auto i = 1u; i < space->sizes.size(); ++i) {

    auto sizeSCEV = space->sizes[i - 1];
    auto instIVPair = space->subscriptIVs[i];
    auto inst = instIVPair.first;
    auto iv = instIVPair.second;
    if (!inst) {
      return false;
    }

    /*
     * In case the subscript SCEV was composed by the Delinearization class
     * which might not use ScalarEvolution cache, we fetch the cached SCEV via
     * the matched instruction
     */
    auto subscriptSCEV = SE.getSCEV(inst);
    if (!subscriptSCEV) {
      return false;
    }

    if (isa<SCEVSignExtendExpr>(subscriptSCEV)
        || isa<SCEVTruncateExpr>(subscriptSCEV)
        || isa<SCEVZeroExtendExpr>(subscriptSCEV)) {
      subscriptSCEV = cast<SCEVCastExpr>(subscriptSCEV)->getOperand();
    }

    auto subscriptType = subscriptSCEV->getType();
    if (!isa<IntegerType>(subscriptType)) {
      return false;
    }

    // sizeSCEV->print(errs() << "Checking if bounded by 0 and ");
    // subscriptSCEV->print(errs() << ", Subscript " << i << ": ");
    // inst->print(errs() << "\tInst: ");
    // errs() << "\n";
    // if (auto ar = dyn_cast<SCEVAddRecExpr>(subscriptSCEV)) {
    //   iv->getLoopEntryPHI()->getParent()->printAsOperand(errs() << "\tfrom
    //   loop: "); errs() << "\n";
    // }
    // errs() << "\tEqual to instruction SCEV: " << (subscriptSCEV ==
    // SE.getSCEV(inst)) << "\n";

    // if (SE.isKnownPredicate(looseLowerBoundPred, subscriptSCEV, zeroSCEV)
    //   && SE.isKnownPredicate(strictUpperBoundPred, subscriptSCEV, sizeSCEV))
    //   continue;

    if (iv && iv->isIVInstruction(inst) && isa<SCEVAddRecExpr>(subscriptSCEV)) {
      auto subscriptRecSCEV = cast<SCEVAddRecExpr>(subscriptSCEV);
      auto loopEntryPHI = iv->getLoopEntryPHI();
      auto loopEntryPHISCEV = cast<SCEVAddRecExpr>(SE.getSCEV(loopEntryPHI));

      // loopEntryPHISCEV->print(errs() << "Loop entry SCEV: "); errs() << "\n";
      // subscriptRecSCEV->print(errs() << "Subscript SCEV: "); errs() << "\n";

      /*
       * If the step recurrence is negative and the AddRecExpr starts at the
       * size - 1, it is bounded
       */
      auto stepSCEV = subscriptRecSCEV->getStepRecurrence(SE);
      auto constantStepSCEV = dyn_cast<SCEVConstant>(stepSCEV);
      if (constantStepSCEV && constantStepSCEV->getValue()->isNegative()) {
        // TODO:
      }

      /*
       * If the AddRecExpr is exactly that of the loop entry PHI for the IV, and
       * that IV's attribution identifies an exit condition value equal to the
       * size, it is bounded
       */
      if (scevsMatch(subscriptRecSCEV->getStart(), loopEntryPHISCEV->getStart())
          && scevsMatch(subscriptRecSCEV->getStepRecurrence(SE),
                        loopEntryPHISCEV->getStepRecurrence(SE))) {
        auto loopHeader = loopEntryPHI->getParent();
        auto loopStructure =
            this->loops->getInnermostLoopThatContains(loopHeader);
        auto attr = ivManager.getLoopGoverningInductionVariable(*loopStructure);

        // attr->getInductionVariable().getLoopEntryPHI()->print(errs() << "ATTR
        // PHI: "); errs() << "\n";

        if (attr != nullptr && iv == attr->getInductionVariable()) {
          if (constantStepSCEV && !constantStepSCEV->getValue()->isNegative()) {
            auto conditionValue = attr->getExitConditionValue();
            auto cmpInst =
                attr->getHeaderCompareInstructionToComputeExitCondition();
            auto predicate = cmpInst->getPredicate();

            auto exitBlock = attr->getExitBlockFromHeader();
            BasicBlock *falseSuccessor = *(++succ_begin(loopHeader));
            bool exitOnFalse = exitBlock == falseSuccessor;
            bool isConditionLHS = cmpInst->getOperand(0) == conditionValue;

            // errs() << "\t\tEOF: " << exitOnFalse << "\n";
            // cmpInst->print(errs() << "\t\tCmpI: "); errs() << "\n";
            // SE.getSCEV(conditionValue)->print(errs() << "\t\tCondition value
            // SCEV: "); errs() << "\n";

            if (predicate == ICmpInst::Predicate::ICMP_ULE
                || predicate == ICmpInst::Predicate::ICMP_SLE) {
              predicate = ICmpInst::Predicate::ICMP_UGT;
              isConditionLHS = !isConditionLHS;
            } else if (predicate == ICmpInst::Predicate::ICMP_UGE
                       || predicate == ICmpInst::Predicate::ICMP_SGE) {
              predicate = ICmpInst::Predicate::ICMP_ULT;
              isConditionLHS = !isConditionLHS;
            }

            bool isUpperBoundedByEq =
                (!exitOnFalse && predicate == ICmpInst::Predicate::ICMP_EQ)
                || (exitOnFalse && predicate == ICmpInst::Predicate::ICMP_NE);
            bool isUpperBoundedByLT =
                exitOnFalse && !isConditionLHS
                && (predicate == ICmpInst::Predicate::ICMP_ULT
                    || predicate == ICmpInst::Predicate::ICMP_SLT);
            bool isUpperBoundedByFlippedGT =
                exitOnFalse && isConditionLHS
                && (predicate == ICmpInst::Predicate::ICMP_UGT
                    || predicate == ICmpInst::Predicate::ICMP_SGT);
            bool isUpperBounded = isUpperBoundedByEq || isUpperBoundedByLT
                                  || isUpperBoundedByFlippedGT;

            if (isUpperBounded) {
              auto conditionSCEVBase = SE.getSCEV(conditionValue);
              auto sizeSCEVBase = sizeSCEV;
              auto opI = conditionSCEVBase;
              if (isa<SCEVSignExtendExpr>(opI) || isa<SCEVTruncateExpr>(opI)
                  || isa<SCEVZeroExtendExpr>(opI)) {
                conditionSCEVBase = cast<SCEVCastExpr>(opI)->getOperand();
              }
              opI = sizeSCEVBase;
              if (isa<SCEVSignExtendExpr>(opI) || isa<SCEVTruncateExpr>(opI)
                  || isa<SCEVZeroExtendExpr>(opI)) {
                sizeSCEVBase = cast<SCEVCastExpr>(opI)->getOperand();
              }

              // conditionSCEVBase->print(errs() << "Condition SCEV "); errs()
              // << "\n"; sizeSCEVBase->print(errs() << "Size SCEV "); errs() <<
              // "\n";

              // Check if the exit condition = dimension size, i.e. there is no
              // spillover
              if (conditionSCEVBase == sizeSCEVBase)
                continue;

              // Check if the exit condition < dimension size (allowed because
              // the step size is positive)
              if (auto conditionOffsetSCEV =
                      dyn_cast<SCEVAddExpr>(conditionSCEVBase)) {
                if (conditionOffsetSCEV->getNumOperands() == 2) {
                  auto lhsSCEV = conditionOffsetSCEV->getOperand(0);
                  auto rhsSCEV = conditionOffsetSCEV->getOperand(1);
                  if ((lhsSCEV == sizeSCEVBase) ^ (rhsSCEV == sizeSCEVBase)) {
                    auto otherHandSideSCEV =
                        lhsSCEV == sizeSCEVBase ? rhsSCEV : lhsSCEV;
                    if (auto constOffsetSCEV =
                            dyn_cast<SCEVConstant>(otherHandSideSCEV)) {
                      if (constOffsetSCEV->getValue()->isNegative()) {
                        continue;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    return false;
  }

  // errs() << "Is bounded\n";
  return true;
}

} // namespace arcana::noelle
