/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/tools/SCEVSimplification.hpp"

using namespace llvm;
using namespace llvm::noelle;

SCEVSimplification::SCEVSimplification (Noelle &noelle)
  : noelle{noelle} {
  auto fm = noelle.getFunctionsManager();
  auto M = fm->getEntryFunction()->getParent();
  auto &cxt = M->getContext();
  auto &dataLayout = M->getDataLayout();
  this->ptrSizeInBits = dataLayout.getPointerSizeInBits();
  this->intTypeForPtrSize = IntegerType::get(cxt, this->ptrSizeInBits);
}

bool SCEVSimplification::simplifyLoopGoverningIVGuards (
  LoopDependenceInfo const &LDI,
  ScalarEvolution &SE
) {

  if (noelle.getVerbosity() != Verbosity::Disabled) {
    errs() << "SCEVSimplification: Start trying to simplify loop governing IV condition\n";
  }

  /*
   * Fetch the information about the loop.
   */
  auto rootLoop = LDI.getLoopStructure();
  auto ivManager = LDI.getInductionVariableManager();

  if (noelle.getVerbosity() != Verbosity::Disabled) {
    errs() << "SCEVSimplification:    Loop " << *rootLoop->getHeader()->getFirstNonPHI() << "\n";
  }

  /*
   * Attempt to find a branch instruction contained within an IV's SCC
   * That IV must have a constant step size for this simplification to be possible
   */
  InductionVariable *loopGoverningIV = nullptr;
  BranchInst *loopGoverningBranchInst = nullptr;
  for (auto iv : ivManager->getInductionVariables(*rootLoop)) {
    auto stepValue = iv->getSingleComputedStepValue();
    if (!stepValue || !isa<ConstantInt>(stepValue)) continue;

    /*
     * NOTE: Investigate whether there could be an IV that is not integer typed.
     * This is likely a mis-classification by InductionVariable
     */
    auto headerPHI = iv->getLoopEntryPHI();
    auto ivInstructions = iv->getAllInstructions();
    if (!headerPHI->getType()->isIntegerTy()) continue;

    /*
     * Fetch the loop governing terminator.
     * NOTE: It should be the only conditional branch in the IV's SCC
     */
    BranchInst *loopGoverningTerminator = nullptr;
    bool hasSingleTerminator = true;
    for (auto internalNodePair : iv->getSCC()->internalNodePairs()) {
      auto value = internalNodePair.first;
      auto br = dyn_cast<BranchInst>(value);
      if (!br || !br->isConditional()) continue;

      if (loopGoverningTerminator) {
        hasSingleTerminator = false;
        break;
      }
      loopGoverningTerminator = br;
    }

    if (!hasSingleTerminator || !loopGoverningTerminator) continue;
    loopGoverningBranchInst = loopGoverningTerminator;
    loopGoverningIV = iv;
    break;
  }

  /*
   * The branch condition must be a CmpInst on an intermediate value of the loop governing IV
   */
  if (!loopGoverningIV) {

    if (noelle.getVerbosity() != Verbosity::Disabled) {
      errs() << "SCEVSimplification: Exit. Loop does not have a governing IV\n";
    }

    return false;
  }
  auto cmpInst = dyn_cast<CmpInst>(loopGoverningBranchInst->getCondition());
  if (!cmpInst) {

    if (noelle.getVerbosity() != Verbosity::Disabled) {
      errs() << "SCEVSimplification: Exit. Governing IV exit condition is not understood\n";
    }

    return false;
  }

  /*
   * Find the intermediate value used in the guard
   */
  auto ivInstructions = loopGoverningIV->getAllInstructions();
  auto opL = cmpInst->getOperand(0);
  auto opR = cmpInst->getOperand(1);
  auto isOpLHSAnIntermediate = isa<Instruction>(opL)
    && ivInstructions.find(cast<Instruction>(opL)) != ivInstructions.end();
  auto isOpRHSAnIntermediate = isa<Instruction>(opR)
    && ivInstructions.find(cast<Instruction>(opR)) != ivInstructions.end();
  if (!(isOpLHSAnIntermediate ^ isOpRHSAnIntermediate)) {

    if (noelle.getVerbosity() != Verbosity::Disabled) {
      errs() << "SCEVSimplification: Exit. Governing IV exit CmpInst is not understood\n";
    }

    return false;
  }

  /*
   * If it is the loop entry PHI, there is no simplification to do
   */
  auto intermediateValueUsedInCompare = cast<Instruction>(isOpLHSAnIntermediate ? opL : opR);
  auto loopEntryPHI = loopGoverningIV->getLoopEntryPHI();
  if (intermediateValueUsedInCompare == loopEntryPHI) {

    if (noelle.getVerbosity() != Verbosity::Disabled) {
      errs() << "SCEVSimplification: Exit. Governing IV exit CmpInst is already comparing against loop entry PHI\n";
    }

    return false;
  }

  /*
   * Determine the step offset between the intermediate and the loop entry PHI
   */
  auto loopEntryPHIStartSCEV = cast<SCEVAddRecExpr>(SE.getSCEV(loopEntryPHI))->getStart();
  auto intermediateStartSCEV = cast<SCEVAddRecExpr>(SE.getSCEV(intermediateValueUsedInCompare))->getStart();
  auto offsetSCEV = getOffsetBetween(SE, loopEntryPHIStartSCEV, intermediateStartSCEV);
  if (!offsetSCEV) {

    if (noelle.getVerbosity() != Verbosity::Disabled) {
      errs() << "SCEVSimplification: Exit. Governing IV exit CmpInst offset value from loop entry PHI is not understood\n";
    }

    return false;
  }

  Value *offsetValue;
  if (auto constSCEV = dyn_cast<SCEVConstant>(offsetSCEV)) {
    offsetValue = constSCEV->getValue();
  } else if (auto valueSCEV = dyn_cast<SCEVUnknown>(offsetSCEV)) {
    offsetValue = valueSCEV->getValue();
  } else {

    /*
     * TODO: Handle fetching values for cast and nary SCEVs
     */

    if (noelle.getVerbosity() != Verbosity::Disabled) {
      errs() << "SCEVSimplification: Exit. Governing IV exit CmpInst offset SCEV from loop entry PHI is not understood\n";
    }

    return false;
  }

  /*
   * Subtract the step offset from the condition used in loop guard
   * Replace comparison on intermediate with a comparison on the loop entry PHI
   */
  IRBuilder<> loopEntryBuilder(cmpInst);
  auto ivOp = isOpLHSAnIntermediate ? 0 : 1;
  auto conditionValueOp = isOpLHSAnIntermediate ? 1 : 0;
  auto conditionValue = cmpInst->getOperand(conditionValueOp);
  auto adjustedConditionValue = loopEntryBuilder.CreateSub(conditionValue, offsetValue);
  cmpInst->setOperand(ivOp, loopEntryPHI);
  cmpInst->setOperand(conditionValueOp, adjustedConditionValue);

  if (noelle.getVerbosity() != Verbosity::Disabled) {
    cmpInst->print(errs() << "SCEVSimplification: Exit. Simplified CmpInst to use loop entry PHI: "); errs() << "\n";
  }

  return true;
}

/*
 * TODO: Find a LLVM solution for this. Don't try to re-invent the wheel
 */
const SCEV *SCEVSimplification::getOffsetBetween (ScalarEvolution &SE, const SCEV *startSCEV, const SCEV *intermediateSCEV) {
  if (auto intermediateConstSCEV = dyn_cast<SCEVConstant>(intermediateSCEV)) {
    auto startConstSCEV = dyn_cast<SCEVConstant>(startSCEV);
    if (!startConstSCEV) return nullptr;

    auto startConst = startConstSCEV->getValue()->getSExtValue();
    auto intermediateConst = intermediateConstSCEV->getValue()->getSExtValue();
    return SE.getConstant(startSCEV->getType(), intermediateConst - startConst, true);
  }

  auto addSCEV = dyn_cast<SCEVAddExpr>(intermediateSCEV);
  if (!addSCEV || addSCEV->getNumOperands() != 2) return nullptr;
  auto lhs = addSCEV->getOperand(0);
  auto rhs = addSCEV->getOperand(1);
  if (!(lhs == startSCEV ^ rhs == startSCEV)) return nullptr;

  auto offset = lhs == startSCEV ? rhs : lhs;
  return offset;
}

bool SCEVSimplification::simplifyIVRelatedSCEVs (
  LoopDependenceInfo const &LDI
) {
  auto rootLoop = LDI.getLoopHierarchyStructures();
  auto invariantManager = LDI.getInvariantManager();
  auto ivManager = LDI.getInductionVariableManager();
  return simplifyIVRelatedSCEVs(rootLoop, invariantManager, ivManager);
}

bool SCEVSimplification::simplifyIVRelatedSCEVs (
  StayConnectedNestedLoopForestNode *rootLoopNode, 
  InvariantManager *invariantManager,
  InductionVariableManager *ivManager
) {
  if (noelle.getVerbosity() != Verbosity::Disabled) {
    errs() << "SCEVSimplification:  Start\n";
  }

  /*
   * Fetch the loop
   */
  assert(rootLoopNode != nullptr);
  auto rootLoop = rootLoopNode->getLoop();

  IVCachedInfo ivCache;
  this->cacheIVInfo(ivCache, rootLoopNode, ivManager);
  searchForInstructionsDerivedFromMultipleIVs(ivCache, rootLoop, invariantManager);

  /*
  for (auto instIVPair : ivCache.ivByInstruction) {
    instIVPair.first->print(errs() << "IV instruction: "); errs() << "\n";
  }
  for (auto derivedI : ivCache.instsDerivedFromMultipleIVs) {
    derivedI->print(errs() << "I from multiple IVs: "); errs() << "\n";
  }
  */

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
        if (!gep->hasIndices()) continue;
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
  std::unordered_set<GEPIndexDerivation *> validGepsToUpCast;
  for (auto gep : geps) {
    auto gepDerivation = new SCEVSimplification::GEPIndexDerivation{gep, rootLoop, invariantManager, ivCache};
    if (!isUpCastPossible(gepDerivation, rootLoop, *invariantManager)) {
      delete gepDerivation;
      continue;
    }
    validGepsToUpCast.insert(gepDerivation);
  }

  bool modified = upCastIVRelatedInstructionsDerivingGEP(ivCache, rootLoopNode, ivManager, invariantManager, validGepsToUpCast);

  for (auto gepDerivation : validGepsToUpCast) {
    delete gepDerivation;
  }

  return modified;
}

void SCEVSimplification::cacheIVInfo (
  IVCachedInfo &ivCache, 
  StayConnectedNestedLoopForestNode *rootLoopNode, 
  InductionVariableManager *ivManager
  ){
  
  /*
   * Fetch the loop
   */
  assert(rootLoopNode != nullptr);
  auto rootLoop = rootLoopNode->getLoop();

  /*
   * Detect all loop governing IVs across the nested loop structure
   * Cache IV information on them
   */
  auto allLoops = rootLoopNode->getLoops();
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
  IVCachedInfo &ivCache,
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
  IVCachedInfo &ivCache,
  StayConnectedNestedLoopForestNode *rootLoopNode, 
  InductionVariableManager *ivManager,
  InvariantManager *invariantManager,
  std::unordered_set<GEPIndexDerivation *> gepDerivations
) {

  /*
   * Fetch the loop.
   */
  assert(rootLoopNode != nullptr);
  auto rootLoop = rootLoopNode->getLoop();

  std::unordered_map<BasicBlock *, LoopStructure *> headerToLoopMap;
  auto rootLoopHeader = rootLoop->getHeader();
  headerToLoopMap.insert(std::make_pair(rootLoopHeader, rootLoop));
  for (auto subLoop : rootLoopNode->getLoops()){
    if (subLoop == rootLoop){
      continue ;
    }
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

    /*
     * Only convert instructions of the wrong size
     */
    auto typeSize = inst->getType()->getIntegerBitWidth();
    if (typeSize == this->ptrSizeInBits) return;

    if (auto phi = dyn_cast<PHINode>(inst)) {

      /*
       * Only convert PHIs of the wrong size
       */
      if (typeSize == this->ptrSizeInBits) return;

      phisToConvert.insert(phi);
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
      if (invariant->getType()->getIntegerBitWidth() == this->ptrSizeInBits) continue;
      loopInvariantsToConvert.insert(invariant);
    }
  }

  /*
  for (auto invariant : loopInvariantsToConvert) {
    invariant->print(errs() << "Invariant to convert: "); errs() << "\n";
  }
  for (auto phi : phisToConvert) {
    phi->print(errs() << "PHI to convert: "); errs() << "\n";
  }
  for (auto nonPHI : nonPHIsToConvert) {
    nonPHI->print(errs() << "Non phi to convert: "); errs() << "\n";
  }
  for (auto inst : castsToRemove) {
    inst->print(errs() << "Cast to remove: "); errs() << "\n";
  }
  */

  if (phisToConvert.size() == 0 && nonPHIsToConvert.size() == 0 && castsToRemove.size() == 0) {
    return false;
  }

  /*
   * Immediately remove all GEP's uses and transitive uses of truncations
   * If that leaves the truncation use-less, erase it
   */
  for (auto obsoleteCast : castsToRemove) {
    auto isUsedOtherThanByGEP = false;
    auto castedValue = obsoleteCast->getOperand(0);

    std::unordered_set<User *> castUsers{obsoleteCast->user_begin(), obsoleteCast->user_end()};
    for (auto user : castUsers) {
      auto userInst = dyn_cast<Instruction>(user);
      if (!userInst) continue;

      auto isUsedByGEP = false;
      for (auto gepDerivation : gepDerivations) {
        if (gepDerivation->gep == userInst
          || gepDerivation->ivDerivingInstructions.find(userInst) != gepDerivation->ivDerivingInstructions.end()) {
          isUsedByGEP = true;
          break;
        }
      }

      if (!isUsedByGEP) {
        isUsedOtherThanByGEP = true;
        continue;
      }

      user->replaceUsesOfWith(obsoleteCast, castedValue);
    }

    if (isUsedOtherThanByGEP) {
      continue;
    }
    ivCache.ivByInstruction.erase(obsoleteCast);
    obsoleteCast->eraseFromParent();
  }

  /*
   * Build a map from old to new typed values
   * First invariants, then PHIs, then a queue of instructions that keeps
   * searching for the next instruction that can be created (whose operands
   * all have been created already)
   */
  std::unordered_map<Value *, Value *> oldToNewTypedMap;

  /*
   * Insert casts on invariants and replace uses
   */
  auto preheaderBlock = rootLoop->getPreHeader();
  IRBuilder<> preheaderBuilder(preheaderBlock->getTerminator());
  const bool isSigned = true;
  for (auto invariant : loopInvariantsToConvert) {
    Value *castedInvariant;
    if (auto invariantInst = dyn_cast<Instruction>(invariant)) {
      IRBuilder<> builder(invariantInst->getNextNode());
      castedInvariant = builder.CreateIntCast(invariant, this->intTypeForPtrSize, isSigned);
    } else {
      castedInvariant = preheaderBuilder.CreateIntCast(invariant, this->intTypeForPtrSize, isSigned);
    }

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

  auto tryAndMapOldOpToNewOp = [&](Value *oldTypedOp) -> Value * {

    /*
     * There won't be an entry in the map for instructions not needing a conversion
     */
    auto oldTypeSizeInBits = oldTypedOp->getType()->getIntegerBitWidth();
    if (oldTypeSizeInBits == this->ptrSizeInBits) return oldTypedOp;

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
       * Create a copy pointing to newly typed operands
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

  /*
   * Catch all users of effected instructions that need to use a truncation of the up-casted instructions
   */
  std::unordered_map<Instruction *, Instruction *> upCastedToTruncatedInstMap;
  auto truncateUpCastedValueForUsersOf = [&](
    Instruction *originalI,
    Instruction *upCastedI
  ) -> void {

    // originalI->print(errs() << "Original instruction reviewing users of: "); errs() << "\n";

    std::unordered_set<User *> allUsers(originalI->user_begin(), originalI->user_end());
    for (auto user : allUsers) {

      // user->print(errs() << "\tAddressing user: "); errs() << "\n";

      /*
       * Prevent creating a truncation for an instruction already converted
       */
      if (oldToNewTypedMap.find(user) != oldToNewTypedMap.end()) continue;

      /*
       * Prevent creating a cast from the same type to the same type
       */
      if (auto cast = dyn_cast<CastInst>(user)) {
        if (user->getType() == intTypeForPtrSize) {
          cast->replaceAllUsesWith(upCastedI);
          cast->eraseFromParent();
          continue;
        }
      }

      /*
       * If no truncation is needed, as the up-casted type matches the user type,
       * just use the up-casted instruction
       */
      if (user->getType() == intTypeForPtrSize) {
        user->replaceUsesOfWith(originalI, upCastedI);
        continue;
      }

      // errs() << "Not the same integer type nor already converted\n";

      /*
       * Truncate the up-casted instruction to match the user's type
       */
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

  for (auto oldInst : oldInstructionsToDelete) {
    ivCache.ivByInstruction.erase(oldInst);
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

  // errs() << "Is derived\n";

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

  auto resultElementType = gep->getSourceElementType();
  if (!resultElementType->isDoubleTy() &&
    !resultElementType->isFloatingPointTy() &&
    !resultElementType->isIntegerTy()) {
    return false;
  }

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
  auto isValidOperationWhenUpCasted = [&](Instruction *inst) -> bool {

    // inst->print(errs() << "Checking validity of: "); errs() << "\n";

    auto opCode = inst->getOpcode();
    if (opCode != Instruction::Shl && opCode != Instruction::LShr && opCode != Instruction::AShr) {

      /*
       * Ensure non-shifting instructions do not operate on truncated bit widths < MIN_BIT_SIZE
       */
      return inst->getType()->getIntegerBitWidth() >= MIN_BIT_SIZE ;

    } else if (isPartOfShlShrTruncationPair(inst)) {

      /*
       * Ensure the number of bits shifted doesn't reduce the bit width to < MIN_BIT_SIZE
       */
      auto bitsShiftedValue = inst->getOperand(1);
      auto bitsShiftedConst = dyn_cast<ConstantInt>(bitsShiftedValue);
      if (!bitsShiftedConst) return false;
      auto bitsShifted = bitsShiftedConst->getValue().getSExtValue();
      if (inst->getType()->getIntegerBitWidth() - bitsShifted < MIN_BIT_SIZE) return false;

      return true;
    }

    return false;
  };

  for (auto inst : gepDerivation->ivDerivingInstructions) {
    if (!isValidOperationWhenUpCasted(inst)) return false;
  }

  // errs() << "Can up cast\n";

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
    if (shr->getOpcode() != Instruction::LShr && shr->getOpcode() != Instruction::AShr) return false;
  } else if (opCode == Instruction::LShr || opCode == Instruction::AShr) {
    shr = I;
    auto opV = I->getOperand(0);
    shl = dyn_cast<Instruction>(opV);
    if (shl->getOpcode() != Instruction::Shl) return false;
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

bool SCEVSimplification::simplifyConstantPHIs (
  LoopDependenceInfo const &LDI
) {
  auto modified = false;

  /*
   * Fetch the loop information.
   */
  auto loopStructure = LDI.getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto loopPreheader = loopStructure->getPreHeader();

  /*
   * Fetch all PHIs of the header of the loop.
   */
  std::unordered_set<PHINode *> headerPHIs;
  for (auto &headerPHI : loopHeader->phis()) {
    headerPHIs.insert(&headerPHI);
  }

  /*
   * Identify the PHIs that can be removed.
   */
  std::unordered_set<PHINode *> removedPHIs;
  for (auto headerPHI : headerPHIs) {
    if (removedPHIs.find(headerPHI) != removedPHIs.end()) {
      continue;
    }

    /*
     * Fetch the live-in value of the current PHI.
     */
    auto liveInValue = headerPHI->getIncomingValueForBlock(loopPreheader);

    auto isPHIPropagation = true;
    std::queue<PHINode *> dependentTraversal;
    std::unordered_set<PHINode *> phiCycle;
    dependentTraversal.push(headerPHI);
    phiCycle.insert(headerPHI);
    while (!dependentTraversal.empty()) {
      auto dependentPHI = dependentTraversal.front();
      dependentTraversal.pop();

      for (auto idx = 0; idx < dependentPHI->getNumIncomingValues(); ++idx) {
        auto incomingValue = dependentPHI->getIncomingValue(idx);
        if (incomingValue) {
          if (liveInValue == incomingValue) continue;
          if (auto incomingPHI = dyn_cast<PHINode>(incomingValue)) {
            if (phiCycle.find(incomingPHI) != phiCycle.end()) continue;
            dependentTraversal.push(incomingPHI);
            phiCycle.insert(incomingPHI);
            continue;
          }
        }

        isPHIPropagation = false;
        break;
      }
      if (!isPHIPropagation) break;
    }
    if (!isPHIPropagation) continue;

    if (noelle.getVerbosity() >= Verbosity::Maximal) {
      headerPHI->print(errs() << "SCEVSimplification: Removing loop entry PHI (part of PHI-only propagation): "); errs() << "\n";
      for (auto phi : phiCycle) {
        phi->print(errs() << "SCEVSimplification: \tRemoving PHI (part of PHI-only propagation): "); errs() << "\n";
      }
    }

    /*
     * Identify whether the live in value just gets propagated between PHIs in the SCC
     * without the PHIs ever changing value
     */
    auto isConstantPropagation = true;
    for (auto phi : phiCycle) {
      for (auto idx = 0; idx < phi->getNumIncomingValues(); ++idx) {
        auto incomingValue = phi->getIncomingValue(idx);
        if (liveInValue == incomingValue) continue;

        if (auto incomingPHI = dyn_cast<PHINode>(incomingValue)) {
          if (phiCycle.find(incomingPHI) != phiCycle.end()) continue;
        } 

        isConstantPropagation = false;
        break;
      }
      if (!isConstantPropagation) break;
    }
    if (!isConstantPropagation) continue;

    for (auto phi : phiCycle) {
      std::unordered_set<User *> nonCycleUsers;
      for (auto user : phi->users()) {
        if (auto phi = dyn_cast<PHINode>(user)) {
          if (phiCycle.find(phi) != phiCycle.end()) continue;
        }
        nonCycleUsers.insert(user);
      }

      for (auto nonCycleUser : nonCycleUsers) {
        nonCycleUser->replaceUsesOfWith(phi, liveInValue);
      }
    }

    /*
     * Delete PHI instructions
     */
    for (auto phi : phiCycle) {
      if (removedPHIs.find(phi) != removedPHIs.end()){
        continue ;
      }
      for (auto i=0; i < phi->getNumIncomingValues(); i++){
        phi->setIncomingValue(i, UndefValue::get(phi->getType()));
      }
    }
    for (auto phi : phiCycle) {
      if (removedPHIs.find(phi) != removedPHIs.end()){
        continue ;
      }
      phi->eraseFromParent();
      removedPHIs.insert(phi);
    }

    modified = true;
  }

  return modified;
}
