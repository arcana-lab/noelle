/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SCEVSimplification.hpp"

using namespace llvm;

SCEVSimplification::SCEVSimplification (Noelle &noelle)
  : noelle{noelle} {
  auto M = noelle.getEntryFunction()->getParent();
  auto &cxt = M->getContext();
  auto &dataLayout = M->getDataLayout();
  this->ptrSizeInBits = dataLayout.getPointerSizeInBits();
  this->intTypeForPtrSize = IntegerType::get(cxt, this->ptrSizeInBits);
}

bool SCEVSimplification::simplifyIVRelatedSCEVs (
  LoopDependenceInfo const &LDI
) {
  return false;

  auto rootLoop = LDI.getLoopStructure();
  auto invariantManager = LDI.getInvariantManager();
  auto ivManager = LDI.getInductionVariableManager();
  return simplifyIVRelatedSCEVs(rootLoop, invariantManager, ivManager);
}

bool SCEVSimplification::simplifyIVRelatedSCEVs (
  LoopStructure *rootLoop,
  InvariantManager *invariantManager,
  InductionVariableManager *ivManager
) {

  if (noelle.getVerbosity() >= Verbosity::Maximal) {
    errs() << "SCEVSimplification:  Start\n";
  }

  cacheIVInfo(rootLoop, ivManager);
  searchForInstructionsDerivedFromMultipleIVs(rootLoop, invariantManager);

  // for (auto instIVPair : ivCache.ivByInstruction) {
  //   instIVPair.first->print(errs() << "IV instruction: "); errs() << "\n";
  // }
  // for (auto derivedI : ivCache.instsDerivedFromMultipleIVs) {
  //   derivedI->print(errs() << "I from multiple IVs: "); errs() << "\n";
  // }

  /*
   * Identify all GEPs to loads or stores within the loop
   */
  std::unordered_set<GetElementPtrInst *> geps;
  for (auto B : rootLoop->getBasicBlocks()) {
    for (auto &I : *B) {
      if (auto gep = dyn_cast<GetElementPtrInst>(&I)) {

        /*
         * Spot checks before further examining:
         * 1) Ensure the indices are integer typed
         * 2) TODO, add more to make this enabler efficient
         */
        auto index0 = gep->indices().begin()->get();
        auto indexType = index0->getType();
        if (!indexType->isIntegerTy()) continue;

        geps.insert(gep);
      }
    }
  }

  /*
   * Filter out GEPs not derived from loop governing IVs or loop invariants
   * Up cast GEP derivations whenever the IV integer size is smaller than the pointer size
   */
  bool modified = false;
  std::unordered_set<GEPIndexDerivation *> validGepsToUpCast;
  for (auto gep : geps) {
    auto gepDerivation = new SCEVSimplification::GEPIndexDerivation{gep, rootLoop, invariantManager, ivCache};
    if (!isUpCastPossible(gepDerivation, rootLoop, *invariantManager)) {
      delete gepDerivation;
      continue;
    }
    validGepsToUpCast.insert(gepDerivation);
  }

  upCastIVRelatedInstructionsDerivingGEP(rootLoop, ivManager, invariantManager, validGepsToUpCast);

  for (auto gepDerivation : validGepsToUpCast) {
    delete gepDerivation;
  }

  return true;
}

void SCEVSimplification::cacheIVInfo (LoopStructure *rootLoop, InductionVariableManager *ivManager) {

  /*
   * Detect all loop governing IVs across the nested loop structure
   * Cache IV information on them
   */
  auto allLoops = rootLoop->getDescendants();
  allLoops.insert(rootLoop);
  for (auto loop : allLoops) {
    auto loopGoverningIVAttr = ivManager->getLoopGoverningIVAttribution(*loop);
    if (!loopGoverningIVAttr) continue;

    auto loopGoverningIV = &loopGoverningIVAttr->getInductionVariable();
    ivCache.loopGoverningAttrByIV.insert(std::make_pair(loopGoverningIV, loopGoverningIVAttr));

    for (auto inst : loopGoverningIV->getAllInstructions()) {
      ivCache.ivByInstruction.insert(
        std::make_pair(inst, loopGoverningIV)
      );
    }
    for (auto inst : loopGoverningIV->getDerivedSCEVInstructions()) {
      ivCache.ivByInstruction.insert(
        std::make_pair(inst, loopGoverningIV)
      );
    }
  }
}

/*
 * REFACTOR: Notice the similarity between this and the InductionVariable derived instruction search
 */
void SCEVSimplification::searchForInstructionsDerivedFromMultipleIVs (
  LoopStructure *rootLoop,
  InvariantManager *invariantManager
) {

  std::unordered_set<Instruction *> checked;
  std::function<bool(Instruction *)> checkIfDerived;
  checkIfDerived = [&](Instruction *I) -> bool {

    /*
     * Check the cache of confirmed derived values,
     * and then what we have already traversed to prevent traversing a cycle
     */
    if (ivCache.ivByInstruction.find(I) != ivCache.ivByInstruction.end()) {
      return true;
    }
    if (ivCache.instsDerivedFromMultipleIVs.find(I) != ivCache.instsDerivedFromMultipleIVs.end()) {
      return true;
    }
    if (checked.find(I) != checked.end()) {
      return false;
    }
    checked.insert(I);

    // I->print(errs() << "Derived check: " << (isa<CastInst>(I) || I->isBinaryOp()) << ": "); errs() << "\n";

    /*
     * Only check values in the loop
     */
    if (!rootLoop->isIncluded(I)) return false;

    /*
     * We only handle unary/binary operations on IV instructions.
     */
    if (!isa<CastInst>(I) && !I->isBinaryOp()) return false;

    /*
     * Ensure the instruction uses the IV at least once, and only this IV,
     * apart from constants and loop invariants
     */
    bool usesAtLeastOneIVInstruction = false;
    for (auto &use : I->operands()) {
      auto usedValue = use.get();

      if (isa<ConstantInt>(usedValue)) continue;
      if (invariantManager->isLoopInvariant(usedValue)) continue;

      if (auto usedInst = dyn_cast<Instruction>(usedValue)) {
        if (!rootLoop->isIncluded(usedInst)) continue;
        auto isDerivedUse = checkIfDerived(usedInst);
        if (isDerivedUse) {
          usesAtLeastOneIVInstruction = true;
          continue;
        }
      }

      // usedValue->print(errs() << "Doesn't use only derived inst: "); errs() << "\n";

      return false;
    }

    // errs() << "Uses at least one derived: " << usesAtLeastOneIVInstruction << "\n";

    if (!usesAtLeastOneIVInstruction) return false;

    /*
     * Cache the result
     */
    ivCache.instsDerivedFromMultipleIVs.insert(I);

    // I->print(errs() << "Adding: "); errs() << "\n";

    return true;
  };

  std::queue<Instruction *> intermediates;
  std::unordered_set<Instruction *> visited;
  for (auto instIVPair : ivCache.ivByInstruction) {
    auto inst = instIVPair.first;
    intermediates.push(inst);
    visited.insert(inst);
  }

  while (!intermediates.empty()) {
    auto I = intermediates.front();
    intermediates.pop();

    for (auto user : I->users()) {
      if (auto userInst = dyn_cast<Instruction>(user)) {
        if (visited.find(userInst) != visited.end()) continue;
        visited.insert(userInst);

        /*
         * If the user isn't derived, do not continue traversing users
         */
        if (!checkIfDerived(userInst)) continue;
        intermediates.push(userInst);
      }
    }
  }
}

/*
 * Up cast all collected loop invariants and IV deriving instructions
 * Replace their uses with the casted instruction
 * Remove any truncations now made unnecessary by up casting
 * Remove any shl-ashr pairs that act as truncations
 */
bool SCEVSimplification::upCastIVRelatedInstructionsDerivingGEP (
  LoopStructure *rootLoop,
  InductionVariableManager *ivManager,
  InvariantManager *invariantManager,
  std::unordered_set<GEPIndexDerivation *> gepDerivations
) {

  std::unordered_map<BasicBlock *, LoopStructure *> headerToLoopMap;
  auto rootLoopHeader = rootLoop->getHeader();
  headerToLoopMap.insert(std::make_pair(rootLoopHeader, rootLoop));
  for (auto subLoop : rootLoop->getDescendants()) {
    auto subLoopHeader = subLoop->getHeader();
    headerToLoopMap.insert(std::make_pair(subLoopHeader, subLoop));
  }

  /*
   * Get loop governing IV that will need their loop guards updated
   */
  std::unordered_set<LoopGoverningIVAttribution *> loopGoverningAttrsToUpdate;
  for (auto gepDerivation : gepDerivations) {
    for (auto IV : gepDerivation->derivingIVs) {
      auto header = IV->getLoopEntryPHI()->getParent();
      auto loop = headerToLoopMap.at(header);
      auto loopGoverningAttr = ivManager->getLoopGoverningIVAttribution(*loop);
      if (!loopGoverningAttr) continue;
      auto loopGoverningIV = &loopGoverningAttr->getInductionVariable();
      if (loopGoverningIV != IV) continue;
      loopGoverningAttrsToUpdate.insert(loopGoverningAttr);
    }
  }

  /*
   * Collect IV related instructions that will be effected
   */
  std::unordered_set<Value *> loopInvariantsToConvert;
  std::unordered_set<PHINode *> phisToConvert;
  std::unordered_set<Instruction *> nonPHIsToConvert;
  std::unordered_set<Instruction *> castsToRemove;
  auto collectInstructionToConvert = [&](Instruction *inst) -> void {

    // inst->print(errs() << "Collecting to convert: "); errs() << "\n";

    if (auto phi = dyn_cast<PHINode>(inst)) {
      phisToConvert.insert(phi);
      return;
    }

    /*
      * Remove deriving casts/truncations that will be obsolete after casting up
      */
    if (isa<TruncInst>(inst)
      || isa<ZExtInst>(inst)
      || isa<SExtInst>(inst)
      || isPartOfShlShrTruncationPair(inst)) {
      castsToRemove.insert(inst);
      return;
    }

    nonPHIsToConvert.insert(inst);
  };

  /*
   * Fetch any IV instructions, casts on them, and derived computation
   * Fetch loop governing IV guard condition derivation
   */
  for (auto gepDerivation : gepDerivations) {
    for (auto inst : gepDerivation->ivDerivingInstructions) {
      collectInstructionToConvert(inst);
    }
    for (auto invariant : gepDerivation->loopInvariantsUsed) {
      loopInvariantsToConvert.insert(invariant);
    }
  }

  // for (auto loopGoverningAttr : loopGoverningAttrsToUpdate) {
  //   auto conditionDerivationInsts = loopGoverningAttr->getConditionValueDerivation();
  //   auto conditionValue = loopGoverningAttr->getHeaderCmpInstConditionValue();
  //   if (auto conditionInst = dyn_cast<Instruction>(conditionValue)) {
  //     conditionDerivationInsts.insert(conditionInst);
  //   } else {
  //     loopInvariantsToConvert.insert(conditionValue);
  //   }

  //   /*
  //    * HACK: The IV used in the comparison instruction could itself be a casted instruction
  //    * Remove that instruction if so
  //    * 
  //    * TODO: Really, we should be traversing all used instructions by the comparison
  //    * in search for instructions to convert and casts to remove until we reach loop
  //    * invariants/externals> That would clean up this and the condition derivation nonsense
  //    */
  //   auto cmpInst = loopGoverningAttr->getHeaderCmpInst();
  //   auto cmpLHS = cmpInst->getOperand(0);
  //   auto cmpRHS = cmpInst->getOperand(1);
  //   auto loopGoverningIV = &loopGoverningAttr->getInductionVariable();
  //   auto ivInstUsed = (isa<Instruction>(cmpLHS)
  //     && loopGoverningIV->isIVInstruction(cast<Instruction>(cmpLHS))) ? cmpLHS : cmpRHS;
  //   if (auto cast = dyn_cast<CastInst>(ivInstUsed)) {
  //     castsToRemove.insert(cast);
  //   }

  //   for (auto inst : conditionDerivationInsts) {
  //     if (rootLoop->isIncluded(inst)
  //       && !invariantManager->isLoopInvariant(inst)) {
  //       collectInstructionToConvert(inst);

  //       /*
  //        * Get loop invariants used by the condition value derivation
  //        * NOTE: Since the derivation is all instructions in the loop needed
  //        * to compute the value, a simple search of direct operands on conditions is sufficient
  //        */
  //       for (auto &op : inst->operands()) {
  //         auto opValue = op.get();
  //         auto opInst = dyn_cast<Instruction>(opValue);
  //         if ((opInst && !rootLoop->isIncluded(opInst))
  //           || invariantManager->isLoopInvariant(opValue)) {
  //           loopInvariantsToConvert.insert(opValue);
  //         }
  //       }

  //       loopInvariantsToConvert.insert(inst);
  //     } else {
  //       loopInvariantsToConvert.insert(inst);
  //     }
  //   }
  // }

  // for (auto invariant : loopInvariantsToConvert) {
  //   invariant->print(errs() << "Invariant to convert: "); errs() << "\n";
  // }
  // for (auto phi : phisToConvert) {
  //   phi->print(errs() << "PHI to convert: "); errs() << "\n";
  // }
  // for (auto nonPHI : nonPHIsToConvert) {
  //   nonPHI->print(errs() << "Non phi to convert: "); errs() << "\n";
  // }
  // for (auto inst : castsToRemove) {
  //   inst->print(errs() << "Cast to remove: "); errs() << "\n";
  // }

  /*
   * Build a map from old to new typed values
   * First invariants, then PHIs, then a queue of instructions that keeps
   * searching for the next instruction that can be created (whose operands
   * all have been created already)
   */
  std::unordered_map<Value *, Value *> oldToNewTypedMap;

  /*
   * Insert casts on invariants in the loop preheader and replace uses
   */
  auto preheaderBlock = rootLoop->getPreHeader();
  IRBuilder<> preheaderBuilder(preheaderBlock->getTerminator());
  const bool isSigned = true;
  for (auto invariant : loopInvariantsToConvert) {
    if (invariant->getType()->getIntegerBitWidth() == this->ptrSizeInBits) {
      // invariant->print(errs() << "Invariant not needing conversion: "); errs() << "\n";
      oldToNewTypedMap.insert(std::make_pair(invariant, invariant));
      continue;
    }

    auto castedInvariant = preheaderBuilder.CreateIntCast(invariant, this->intTypeForPtrSize, isSigned);
    // invariant->print(errs() << "Invariant casted: "); errs() << "\n";
    oldToNewTypedMap.insert(std::make_pair(invariant, castedInvariant));
  }

  /*
   * Replace original PHIs with newly typed PHIs, remove casts
   */
  for (auto phi : phisToConvert) {
    IRBuilder<> builder(phi);
    auto numIncomingValues = phi->getNumIncomingValues();
    auto newlyTypedPHI = builder.CreatePHI(intTypeForPtrSize, numIncomingValues);
    oldToNewTypedMap.insert(std::make_pair(phi, newlyTypedPHI));
  }

  std::unordered_set<Instruction *> valuesLeft;
  for (auto nonPHI : nonPHIsToConvert) {
    valuesLeft.insert(nonPHI);
  }
  for (auto cast : castsToRemove) {
    valuesLeft.insert(cast);
  }

  auto tryAndMapOldOpToNewOp = [&](Value *oldTypedOp) -> Value * {
    // oldTypedOp->print(errs() << "\t\tMapping: "); errs() << "\n";
    if (auto constOp = dyn_cast<ConstantInt>(oldTypedOp)) {
      auto constPtrSize = ConstantInt::get(intTypeForPtrSize, constOp->getValue().getSExtValue(), isSigned);
      // constPtrSize->print(errs() << "\t\t\tIs constant: "); errs() << "\n";
      return constPtrSize;
    }

    if (oldToNewTypedMap.find(oldTypedOp) == oldToNewTypedMap.end()) return nullptr;
    // errs() << "\t\t\tIs already mapped\n";
    return oldToNewTypedMap.at(oldTypedOp);
  };

  auto prevValuesLeft = 0;
  while (prevValuesLeft != valuesLeft.size() && valuesLeft.size() > 0) {
    prevValuesLeft = valuesLeft.size();
    std::queue<Instruction *> valuesToConvert{};
    for (auto valueLeft : valuesLeft) {
      valuesToConvert.push(valueLeft);
    }

    while (!valuesToConvert.empty()) {
      auto I = valuesToConvert.front();
      valuesToConvert.pop();

      // I->print(errs() << "Converting: "); errs() << "\n";

      /*
       * Ensure all operands used by this value are already converted
       */
      std::vector<Value *> newTypedOps;
      for (auto &op : I->operands()) {
        auto oldTypedOp = op.get();
        auto newTypedOp = tryAndMapOldOpToNewOp(oldTypedOp);
        if (!newTypedOp) {
          // oldTypedOp->print(errs() << "\tCant find new type value for: "); errs() << "\n";
          break;
        }
        newTypedOps.push_back(newTypedOp);
      }

      bool allOperandsAbleToConvert = newTypedOps.size() == I->getNumOperands();
      // errs() << "\tAble to convert?  " << allOperandsAbleToConvert << "\n";
      if (!allOperandsAbleToConvert) continue;


      /*
       * To remove casts, map the operand being casted to its newly typed operand
       */
      if (castsToRemove.find(I) != castsToRemove.end()) {
        auto valuePreviouslyCasted = I->getOperand(0);
        auto newTypedPreviousValue = oldToNewTypedMap.at(valuePreviouslyCasted);
        oldToNewTypedMap.insert(std::make_pair(I, newTypedPreviousValue));
        valuesLeft.erase(I);
        // I->print(errs() << "\tRemoved: ");
        // newTypedPreviousValue->print(errs() << "\t and will use: "); errs() << "\n";
        continue;
      }

      /*
       * For all other instructions, create a copy pointing to newly typed operands
       */
      auto opCode = I->getOpcode();
      Value *newInst = nullptr;
      IRBuilder<> builder(I);
      if (I->isUnaryOp()) {
        auto unaryOpCode = static_cast<Instruction::UnaryOps>(opCode);
        newInst = builder.CreateUnOp(unaryOpCode, newTypedOps[0]);
      } else if (I->isBinaryOp()) {
        auto binaryOpCode = static_cast<Instruction::BinaryOps>(opCode);
        newInst = builder.CreateBinOp(binaryOpCode, newTypedOps[0], newTypedOps[1]);
      } else {
        assert(false && "SCEVSimplification: instruction being up-casted is not an unary or binary operator!");
      }

      // I->print(errs() << "\tswapping: ");
      // newInst->print(errs() << "\t with inst: "); errs() << "\n";

      oldToNewTypedMap.insert(std::make_pair(I, newInst));
      valuesLeft.erase(I);
    }

    // for (auto valueLeft : valuesLeft) {
    //   valueLeft->print(errs() << "Value left: "); errs() << "\n";
    // }
    // errs() << "----\n";
  }

  assert(valuesLeft.size() == 0 && "SCEVSimplification: failed mid-way in simplifying");

  // auto upCastOrGetMappedUpCast = [&](Value *value) -> Value *{
  //   auto fetchedUpCast = tryAndMapOldOpToNewOp(value);
  //   if (fetchedUpCast) {
  //     return fetchedUpCast;
  //   }

  //   auto inst = dyn_cast<Instruction>(value);
  //   if (!inst) {
  //     auto preHeader = rootLoop->getPreHeader();
  //     IRBuilder<> entry(preHeader->getTerminator());
  //     return entry.CreateIntCast(value, this->intTypeForPtrSize, isSigned);
  //   }

  //   if (!rootLoop->isIncluded(inst)) {
  //     IRBuilder<> builder(inst);
  //     return builder.CreateIntCast(inst, this->intTypeForPtrSize, isSigned);
  //   }

  //   return nullptr;
  // };

  /*
   * Update loop guards with newly typed operands created
   */
  // for (auto loopGoverningAttr : loopGoverningAttrsToUpdate) {
  //   auto cmpInst = loopGoverningAttr->getHeaderCmpInst();
  //   IRBuilder<> builder(cmpInst);
  //   auto predicate = cmpInst->getPredicate();
  //   auto oldTypedLHS = cmpInst->getOperand(0);
  //   auto newTypedLHS = upCastOrGetMappedUpCast(oldTypedLHS);
  //   auto oldTypedRHS = cmpInst->getOperand(1);
  //   auto newTypedRHS = upCastOrGetMappedUpCast(oldTypedRHS);

  //   oldTypedLHS->print(errs() << "Old LHS: "); errs() << "\n";
  //   oldTypedRHS->print(errs() << "Old RHS: "); errs() << "\n";
  //   newTypedLHS->print(errs() << "New LHS: "); errs() << "\n";
  //   newTypedRHS->print(errs() << "New RHS: "); errs() << "\n";

  //   assert(newTypedLHS && newTypedRHS && "Guard of up-casted IV could not be updated");

  //   auto newTypedCmp = builder.CreateICmp(predicate, newTypedLHS, newTypedRHS);
  //   cmpInst->replaceAllUsesWith(newTypedCmp);
  //   cmpInst->eraseFromParent();
  // }

  /*
   * Catch all users of effected instructions that need to use a truncation of the up-casted instructions
   */
  std::unordered_map<Instruction *, Instruction *> upCastedToTruncatedInstMap;
  auto truncateUpCastedValueForUsersOf = [&](
    Instruction *originalI,
    Instruction *upCastedI
  ) -> void {

    originalI->print(errs() << "Original instruction reviewing users of: "); errs() << "\n";

    std::unordered_set<User *> allUsers(originalI->user_begin(), originalI->user_end());
    for (auto user : allUsers) {

      // user->print(errs() << "\tAddressing user: "); errs() << "\n";

      /*
       * Prevent creating a truncation for a cast that will be removed
       * or an instruction already converted
       */
      if (oldToNewTypedMap.find(user) != oldToNewTypedMap.end()) continue;
      if (auto cast = dyn_cast<CastInst>(user)) {
        if (user->getType() == intTypeForPtrSize) {
          cast->replaceAllUsesWith(upCastedI);
          cast->eraseFromParent();
        }
      }

      if (user->getType() == intTypeForPtrSize) {
        user->replaceUsesOfWith(originalI, upCastedI);
        continue;
      }

      // errs() << "Not the same integer type nor already converted\n";

      Instruction * truncatedI = nullptr;
      if (upCastedToTruncatedInstMap.find(upCastedI) == upCastedToTruncatedInstMap.end()) {
        Instruction *afterI = upCastedI->getNextNode();
        assert(afterI && "Cannot up cast terminators");
        if (isa<PHINode>(afterI)) {
          afterI = upCastedI->getParent()->getFirstNonPHIOrDbgOrLifetime();
        }

        IRBuilder<> builder(afterI);
        truncatedI = cast<Instruction>(builder.CreateTrunc(upCastedI, originalI->getType()));
        upCastedToTruncatedInstMap.insert(std::make_pair(upCastedI, truncatedI));
      } else {
        truncatedI = upCastedToTruncatedInstMap.at(upCastedI);
      }

      user->replaceUsesOfWith(originalI, truncatedI);
    }
  };

  for (auto gepDerivation : gepDerivations) {
    for (auto i = 1; i < gepDerivation->gep->getNumOperands(); ++i) {
      auto oldIndexValue = gepDerivation->gep->getOperand(i);
      auto newIndexValue = tryAndMapOldOpToNewOp(oldIndexValue);
      gepDerivation->gep->setOperand(i, newIndexValue);
    }
  }

  std::unordered_set<Instruction *> oldInstructionsToDelete{};
  for (auto oldPHI : phisToConvert) {
    auto newValue = oldToNewTypedMap.at(oldPHI);
    auto newPHI = dyn_cast<PHINode>(newValue);
    assert(newPHI && "Mapping was from non-phi to phi");

    for (auto i = 0; i < oldPHI->getNumIncomingValues(); ++i) {
      auto incomingBlock = oldPHI->getIncomingBlock(i);
      auto oldIncomingValue = oldPHI->getIncomingValue(i);
      auto newIncomingValue = tryAndMapOldOpToNewOp(oldIncomingValue);
      assert(newIncomingValue);
      newPHI->addIncoming(newIncomingValue, incomingBlock);
    }

    truncateUpCastedValueForUsersOf(oldPHI, newPHI);
    oldInstructionsToDelete.insert(oldPHI);
  }
  for (auto oldInst : nonPHIsToConvert) {
    auto newValue = tryAndMapOldOpToNewOp(oldInst);
    assert(newValue);
    auto newInst = dyn_cast<Instruction>(newValue);
    assert(newInst && "Mapping was from non-instruction to instruction");

    truncateUpCastedValueForUsersOf(oldInst, newInst);
    oldInstructionsToDelete.insert(oldInst);
  }
  for (auto obsoleteCast : castsToRemove) {
    oldInstructionsToDelete.insert(obsoleteCast);
  }

  for (auto oldInst : oldInstructionsToDelete) {
    oldInst->eraseFromParent();
  }

  // errs() << "Done\n";
  // rootLoop->getHeader()->getParent()->print(errs() << "FUNCTION:\n"); errs() << "\n";

  return true;
}

SCEVSimplification::GEPIndexDerivation::GEPIndexDerivation (
  GetElementPtrInst *gep,
  LoopStructure *rootLoop,
  InvariantManager *invariantManager,
  IVCachedInfo &ivCache
) : gep{gep}, isDerived{false} {

  // gep->print(errs() << "Checking: "); errs() << "\n";

  /*
   * Queue up to check that all GEP indices have IV derivations
   */
  std::queue<Value *> derivationQueue;
  std::unordered_set<Value *> visited;
  for (auto &indexOp : gep->indices()) {
    auto indexValue = indexOp.get();
    derivationQueue.push(indexValue);
    visited.insert(indexValue);
  }

  while (!derivationQueue.empty()) {
    auto derivingValue = derivationQueue.front();
    derivationQueue.pop();

    // derivingValue->print(errs() << "Queued: "); errs() << "\n";

    if (isa<ConstantInt>(derivingValue)) continue;

    /*
     * If the value is loop invariant, cache and continue
     */
    auto derivingInst = dyn_cast<Instruction>(derivingValue);
    if ((derivingInst && !rootLoop->isIncluded(derivingInst))
      || invariantManager->isLoopInvariant(derivingValue)) {
      this->loopInvariantsUsed.insert(derivingValue);
      continue;
    }

    /*
     * Ensure the value is an instruction associated to the IV
     */
    if (!derivingInst) return ;

    bool isDerivedFromOneIV = ivCache.ivByInstruction.find(derivingInst) !=
      ivCache.ivByInstruction.end();
    bool isDerivedFromManyIV = ivCache.instsDerivedFromMultipleIVs.find(derivingInst) !=
      ivCache.instsDerivedFromMultipleIVs.end();

    // derivingInst->print(errs() << "Deriving I: "); errs() << "\n";

    if (isDerivedFromOneIV) {
      auto derivingIV = ivCache.ivByInstruction.at(derivingInst);
      this->derivingIVs.insert(derivingIV);
      // errs() << "\tIs from 1 IV\n";
    } else if (isDerivedFromManyIV) {
      // errs() << "\tIs from 2+ IV\n";
    } else {
      // errs() << "\t Is not an IV instruction\n";
      return;
    }

    this->ivDerivingInstructions.insert(derivingInst);
    for (auto &op : derivingInst->operands()) {
      auto usedValue = op.get();
      if (visited.find(usedValue) != visited.end()) continue;

      // usedValue->print(errs() << "Used value: "); errs() << "\n";

      visited.insert(usedValue);
      derivationQueue.push(usedValue);
    }
  }

  isDerived = true;
  return;
}

bool SCEVSimplification::isUpCastPossible (
  GEPIndexDerivation *gepDerivation,
  LoopStructure *rootLoop,
  InvariantManager &invariantManager
) const {
  if (!gepDerivation->isDerived) return false;

  auto gep = gepDerivation->gep;

  // gep->print(errs() << "GEP that isn't being accessed as an array pointer: "); errs() << "\n";
  // gep->getType()->print(errs() << "GEP type: "); errs() << "\n";
  // gep->getOperand(0)->print(errs() << "Element: "); errs() << "\n";
  // gep->getOperand(0)->getType()->print(errs() << "Element type: "); errs() << "\n";

  /*
   * Ensure the element being accessed is being accessed as some type of contiguous memory,
   * that is, an access of ptrSizeInBits integer type
   */
  auto firstIdxValue = gep->indices().begin()->get();
  if (firstIdxValue->getType()->getIntegerBitWidth() != ptrSizeInBits) {
    return false;
  }

  /*
   * Ensure the IVs deriving the indices are all a smaller
   * type than the target (pointer size) type
   */
  for (auto IV : gepDerivation->derivingIVs) {
    if (IV->getLoopEntryPHI()->getType()->getIntegerBitWidth() > ptrSizeInBits) return false;
  }

  /*
   * HACK: Ensure that any truncations on loop variants are:
   * from no larger than the pointer size
   * to no smaller than MIN_BIT_SIZE bits
   */
  const int MIN_BIT_SIZE = ptrSizeInBits < 32 ? ptrSizeInBits : 32;
  const int MAX_BIT_SHIFT = ptrSizeInBits - MIN_BIT_SIZE;
  auto isValidOperationWhenUpCasted = [&](Instruction *inst) -> bool {
    auto firstOp = inst->getOperand(0);
    auto srcTy = firstOp->getType();
    auto destTy = inst->getType();
    assert(destTy->isIntegerTy() && srcTy->isIntegerTy());
    if (srcTy->getIntegerBitWidth() < MIN_BIT_SIZE) return false;
    if (destTy->getIntegerBitWidth() < MIN_BIT_SIZE) return false;

    /*
     * Ensure the number of bits shifted doesn't reduce the value bit width below MIN_BIT_SIZE
     */
    if (isPartOfShlShrTruncationPair(inst)) {
      auto bitsShiftedValue = inst->getOperand(1);
      auto bitsShiftedConst = dyn_cast<ConstantInt>(bitsShiftedValue);
      if (!bitsShiftedConst) return false;
      auto bitsShifted = bitsShiftedConst->getValue().getSExtValue();
      if (bitsShifted > MAX_BIT_SHIFT) return false;
    }

    return true;
  };

  for (auto inst : gepDerivation->ivDerivingInstructions) {
    if (!isValidOperationWhenUpCasted(inst)) return false;
  }
  return true;
}

bool SCEVSimplification::isPartOfShlShrTruncationPair (Instruction *I) const {
  Instruction *shl = nullptr;
  Instruction *shr = nullptr;

  /*
   * Fetch the other of the pair
   */
  auto opCode = I->getOpcode();
  if (opCode == Instruction::Shl) {
    shl = I;
    if (!I->hasOneUse()) return false;
    User *user = *(I->user_begin());
    shr = dyn_cast<Instruction>(user);
  } else if (opCode == Instruction::LShr || opCode == Instruction::AShr) {
    shr = I;
    auto opV = I->getOperand(0);
    shl = dyn_cast<Instruction>(opV);
  }

  /*
   * Validate the pair exists, that the potentially fetched Shl is only used by this Shr,
   * and that the bits shifted are the same between the two
   */
  if (!shl || !shr) return false;
  if (!shl->hasOneUse()) return false;
  if (shl->getOperand(1) != shr->getOperand(1)) return false;
  return true;
}