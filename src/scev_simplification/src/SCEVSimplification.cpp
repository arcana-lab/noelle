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

  if (noelle.getVerbosity() >= Verbosity::Maximal) {
    errs() << "SCEVSimplification:  Start\n";
  }

  auto ivManager = LDI.getInductionVariableManager();
  auto invariantManager = LDI.getInvariantManager();
  auto rootLoop = LDI.getLoopStructure();
  cacheIVInfo(rootLoop, ivManager);

  /*
   * Identify all GEPs to loads or stores within the loop
   */
  std::unordered_set<GetElementPtrInst *> geps;
  for (auto B : rootLoop->getBasicBlocks()) {
    for (auto &I : *B) {
      Value *memoryAccessorValue; 
      if (auto store = dyn_cast<StoreInst>(&I)) {
        memoryAccessorValue = store->getPointerOperand();
      } else if (auto load = dyn_cast<LoadInst>(&I)) {
        memoryAccessorValue = load->getPointerOperand();
      } else continue;

      if (auto gep = dyn_cast<GetElementPtrInst>(memoryAccessorValue)) {
        geps.insert(gep);
      }
    }
  }

  /*
   * Filter out GEPs not derived from loop governing IVs or loop invariants
   * Up cast GEP derivations whenever the IV integer size is smaller than the pointer size
   */
  bool modified = false;
  for (auto gep : geps) {
    SCEVSimplification::GEPIndexDerivation gepDerivation{gep, invariantManager, ivCache};
    if (!isUpCastPossible(gepDerivation)) continue;
    upCastIVRelatedInstructionsDerivingGEP(rootLoop, gepDerivation);
    modified = true;
  }

  return modified;
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
      ivCache.ivByInstruction.insert(std::make_pair(inst, loopGoverningIV));
    }
    for (auto inst : loopGoverningIV->getDerivedSCEVInstructions()) {
      ivCache.ivByInstruction.insert(std::make_pair(inst, loopGoverningIV));
    }
  }
}

void SCEVSimplification::upCastIVRelatedInstructionsDerivingGEP (
  LoopStructure *rootLoop,
  SCEVSimplification::GEPIndexDerivation &gepDerivation
) {

  /*
   * Up cast all collected loop invariants and IV deriving instructions
   * Replace their uses with the casted instruction
   * Remove any truncations now made unnecessary by up casting
   * Remove any shl-ashr pairs that act as truncations
   */

  auto &index0 = *gepDerivation.gep->indices().begin();
  assert(index0->getType()->getIntegerBitWidth() == this->ptrSizeInBits
    && "SCEVSimplification: GEP index size is misunderstood");

  /*
   * Insert casts on invariants in the loop preheader and replace uses
   */
  auto preheaderBlock = rootLoop->getPreHeader();
  IRBuilder<> preheaderBuilder(preheaderBlock->getTerminator());
  const bool isSigned = true;
  for (auto invariant : gepDerivation.loopInvariantsUsed) {
    if (invariant->getType()->getIntegerBitWidth() == this->ptrSizeInBits) continue;

    auto castedInvariant = preheaderBuilder.CreateIntCast(invariant, this->intTypeForPtrSize, isSigned);
    std::unordered_set<User *> invariantUsers(invariant->user_begin(), invariant->user_end());
    for (auto user : invariantUsers) {
      if (!gepDerivation.isDerivingInstruction(user)) continue;
      user->replaceUsesOfWith(invariant, castedInvariant);
    }
  }

  /*
   * Collect IV related instructions that will be effected
   */
  std::unordered_set<PHINode *> phisToConvert;
  std::unordered_set<Instruction *> nonPHIsToConvert;
  std::unordered_set<Instruction *> castsToRemove;
  for (auto IV : gepDerivation.derivingIVs) {
    std::unordered_set<Instruction *> instsToConvert;
    for (auto phi : IV->getPHIs()) {
      phisToConvert.insert(phi);
    }
    for (auto nonPHI : IV->getNonPHIIntermediateValues()) {
      nonPHIsToConvert.insert(nonPHI);
    }

    for (auto inst : IV->getDerivedSCEVInstructions()) {
      if (auto phi = dyn_cast<PHINode>(inst)) {
        phisToConvert.insert(phi);
        continue;
      }

      /*
       * Remove deriving casts/truncations that will be obsolete after casting up
       */
      if (isa<TruncInst>(inst) || isa<ZExtInst>(inst) || isa<SExtInst>(inst)) {
        castsToRemove.insert(inst);
        continue;
      }

      instsToConvert.insert(inst);
    }
  }

  /*
   * Replace original PHIs with newly typed PHIs, remove casts
   */

  // std::unordered_map<Instruction *, Instruction *> ivInstToConvertedMap;
  // for (auto inst : instsToConvert) {
  //   auto B = inst->getParent();
  //   IRBuilder<> builder(inst);
  //   Value * placeholder = nullptr;

  //   if (auto phi = dyn_cast<PHINode>(inst)) {
  //     placeholder = builder.CreatePHI(this->intTypeForPtrSize, phi->getNumIncomingValues());
  //   } else {
  //     auto opCode = inst->getOpcode();
  //     if (inst->isUnaryOp()) {
  //       auto unaryOpCode = dyn_cast<Instruction::UnaryOps>(opCode);
  //       placeholder = builder.CreateUnOp(unaryOpCode, inst->getOperand(0));
  //     } else if (inst->isBinaryOp()) {
  //       auto binaryOpCode = dyn_cast<Instruction::BinaryOps>(opCode);
  //       placeholder = builder.CreateBinOp(binaryOpCode, inst->getOperand(0), inst->getOperand(1));
  //     } else {
  //       assert(false && "IV derived instruction is not a PHI, unary, or binary operator!");
  //     }
  //   }

  //   auto placeholderInst = cast<Instruction>(placeholder);
  //   ivInstToConvertedMap.insert(std::make_pair(inst, placeholderInst));
  // }

  /*
   * Catch all users of effected instructions that need to use a truncation of the up-casted instructions
   * This does not include the CmpInst for these loop governing IVs which should be updated itself
   */

  return;
}


SCEVSimplification::GEPIndexDerivation::GEPIndexDerivation (
  GetElementPtrInst *gep,
  InvariantManager *invariantManager,
  IVCachedInfo &ivCache
) : gep{gep}, isDerived{false} {

  /*
   * Identify the GEP itself as a deriving value for symmetry of normalizations
   */
  this->derivingInstructions.insert(gep);

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

    /*
     * If the value is loop invariant, cache and continue
     */
    if (invariantManager->isLoopInvariant(derivingValue)) {
      this->loopInvariantsUsed.insert(derivingValue);
      continue;
    }

    if (isa<ConstantInt>(derivingValue)) continue;

    /*
     * Ensure the value is an instruction associated to the IV
     */
    auto derivingInst = dyn_cast<Instruction>(derivingValue);
    if (!derivingInst) return ;
    if (ivCache.ivByInstruction.find(derivingInst) == ivCache.ivByInstruction.end()) return;

    this->derivingInstructions.insert(derivingInst);
    auto derivingIV = ivCache.ivByInstruction.at(derivingInst);
    this->derivingIVs.insert(derivingIV);

    /*
     * Only traverse up uses if the instruction isn't part of the IV evolution to prevent
     * entering that cycle
     */
    if (derivingIV->isIVInstruction(derivingInst)) continue;
    for (auto &use : derivingValue->uses()) {
      auto usedValue = use.get();
      if (visited.find(usedValue) != visited.end()) continue;

      visited.insert(usedValue);
      derivationQueue.push(usedValue);
    }
  }

  isDerived = true;
  return;
}

bool SCEVSimplification::GEPIndexDerivation::isDerivingInstruction (Value *value) const {
  if (auto inst = dyn_cast<Instruction>(value)) {
    return derivingInstructions.find(inst) != derivingInstructions.end();
  }
  return false;
}

bool SCEVSimplification::isUpCastPossible (
  SCEVSimplification::GEPIndexDerivation &gepDerivation
) const {
  if (!gepDerivation.isDerived) return false;

  /*
   * Ensure the indices are integer typed
   */
  auto index0 = gepDerivation.gep->indices().begin()->get();
  auto indexType = index0->getType();
  if (!indexType->isIntegerTy()) return false;

  /*
   * Ensure the IVs deriving the indices are all guarded by signed comparisons
   */
  for (auto IV : gepDerivation.derivingIVs) {
    if (ivCache.loopGoverningAttrByIV.find(IV) == ivCache.loopGoverningAttrByIV.end()) return false;
    auto attr = ivCache.loopGoverningAttrByIV.at(IV);
    auto cmpInst = attr->getHeaderCmpInst();
    if (cmpInst->isUnsigned()) return false;
  }

  /*
   * Ensure all PHIs have no wrapping ???
   */

  /*
   * Ensure that no casts/truncations exist from/to above the pointer size
   * or from/to below ???
   */
  for (auto inst : gepDerivation.derivingInstructions) {
    if (auto trunc = dyn_cast<TruncInst>(inst)) {
      // trunc->getDestTy()
    }
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