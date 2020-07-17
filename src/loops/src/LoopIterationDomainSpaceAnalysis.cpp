/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "LoopIterationDomainSpaceAnalysis.hpp"

using namespace llvm;

LoopIterationDomainSpaceAnalysis::LoopIterationDomainSpaceAnalysis (
  LoopsSummary &loops,
  InductionVariableManager &ivManager,
  ScalarEvolution &SE
) : loops{loops}, ivManager{ivManager} {

  /*
   * Map IV instructions to SCEVs for quick lookup
   */
  indexIVInstructionSCEVs(SE);
  if (ivInstructionsBySCEV.size() == 0) {
    return ;
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

bool LoopIterationDomainSpaceAnalysis::areInstructionsAccessingDisjointMemoryLocationsBetweenIterations (
  Instruction *I,
  Instruction *J
) const {
  if (!I || !J
    || accessSpaceByInstruction.find(I) == accessSpaceByInstruction.end()
    || accessSpaceByInstruction.find(J) == accessSpaceByInstruction.end()) {
    return false;
  }

  auto accessSpaceI = accessSpaceByInstruction.at(I);
  auto accessSpaceJ = accessSpaceByInstruction.at(J);
  if (nonOverlappingAccessesBetweenIterations.find(accessSpaceI) ==
    nonOverlappingAccessesBetweenIterations.end()) return false;
  if (nonOverlappingAccessesBetweenIterations.find(accessSpaceJ) ==
    nonOverlappingAccessesBetweenIterations.end()) return false;

  return accessSpaceI == accessSpaceJ ||
    isMemoryAccessSpaceEquivalentForTopLoopIVSubscript(accessSpaceI, accessSpaceJ);
}

bool LoopIterationDomainSpaceAnalysis::isMemoryAccessSpaceEquivalentForTopLoopIVSubscript (
  MemoryAccessSpace *space1,
  MemoryAccessSpace *space2
) const {

  if (space1->subscriptIVs.size() == 0) return false;
  if (space1->subscriptIVs.size() != space2->subscriptIVs.size()) return false;

  // space1->memoryAccessor->print(errs() << "Space 1 accessor: "); errs() << "\t";
  // space2->memoryAccessor->print(errs() << "Space 2 accessor: "); errs() << "\n";

  auto getLoopForIV = [&](InductionVariable *iv) -> LoopStructure * {
    if (!iv) return nullptr;
    auto loopEntryPHI = iv->getLoopEntryPHI();
    return loops.getLoop(*loopEntryPHI);
  };

  /*
   * For the memory access spaces to be equivalent per each top loop's iteration,
   * without more extensive analysis, we restrict that all dimensions governed by
   * the top loop's IV must be governed by that IV for BOTH spaces, and governed
   * by the same SCEV derived from that IV
   */
  auto rootLoopStructure = loops.getLoopNestingTreeRoot();
  for (auto subscriptIdx = 0; subscriptIdx < space1->subscriptIVs.size(); ++subscriptIdx) {
    auto iv1 = space1->subscriptIVs[subscriptIdx].second;
    auto iv2 = space2->subscriptIVs[subscriptIdx].second;
    auto loop1 = getLoopForIV(iv1);
    auto loop2 = getLoopForIV(iv2);
    if (rootLoopStructure == loop1 ^ rootLoopStructure == loop2) return false;

    auto scev1 = space1->subscripts[subscriptIdx];
    auto scev2 = space2->subscripts[subscriptIdx];
    if (rootLoopStructure == loop1 && scev1 != scev2) return false;
  }

  return true;
}

void LoopIterationDomainSpaceAnalysis::indexIVInstructionSCEVs (ScalarEvolution &SE) {
  for (auto loop : loops.loops) {
    for (auto iv : ivManager.getInductionVariables(*loop.get())) {
      for (auto inst : iv->getAllInstructions()) {
        if (!SE.isSCEVable(inst->getType())) continue;
        auto scev = SE.getSCEV(inst);

        // scev->getType()->print(errs() << "IV instruction SCEV: ");
        // scev->print(errs() << " ");
        // inst->print(errs() << "\n\tIV I: "); errs() << "\n";

        if (ivInstructionsBySCEV.find(scev) == ivInstructionsBySCEV.end()) {
          ivInstructionsBySCEV.insert(std::make_pair(scev, std::unordered_set<Instruction *>{ inst }));
        } else {
          ivInstructionsBySCEV.at(scev).insert(inst);
        }
        ivsByInstruction.insert(std::make_pair(inst, iv));
      }

      for (auto inst : iv->getDerivedSCEVInstructions()) {
        if (!SE.isSCEVable(inst->getType())) continue;
        auto scev = SE.getSCEV(inst);

        // scev->getType()->print(errs() << "IV derived instruction SCEV: ");
        // scev->print(errs() << " ");
        // inst->print(errs() << "\n\tIV derived I: "); errs() << "\n";

        if (derivedInstructionsFromIVsBySCEV.find(scev) == derivedInstructionsFromIVsBySCEV.end()) {
          derivedInstructionsFromIVsBySCEV.insert(std::make_pair(scev, std::unordered_set<Instruction *>{ inst }));
        } else {
          derivedInstructionsFromIVsBySCEV.at(scev).insert(inst);
        }
        ivsByInstruction.insert(std::make_pair(inst, iv));
      }
    }
  }

  return;
}

void LoopIterationDomainSpaceAnalysis::computeMemoryAccessSpace (ScalarEvolution &SE) {

  std::unordered_set<Instruction *> memoryAccessors{};
  for (auto B : loops.getLoopNestingTreeRoot()->getBasicBlocks()) {
    for (auto &I : *B) {
      Value *memoryAccessorValue; 
      if (auto store = dyn_cast<StoreInst>(&I)) {
        memoryAccessorValue = store->getPointerOperand();
      } else if (auto load = dyn_cast<LoadInst>(&I)) {
        memoryAccessorValue = load->getPointerOperand();
      } else continue;

      if (auto memoryAccessor = dyn_cast<Instruction>(memoryAccessorValue)) {
        memoryAccessors.insert(memoryAccessor);
      }
    }
  }

  for (auto memoryAccessor : memoryAccessors) {
    
    /*
     * Construct memory space object to track this accessor
     */
    auto element = accessSpaces.insert(std::move(
      std::make_unique<LoopIterationDomainSpaceAnalysis::MemoryAccessSpace>(memoryAccessor)
    ));
    auto memAccessSpace = (*element.first).get();
    accessSpaceByInstruction.insert(std::make_pair(memoryAccessor, memAccessSpace));

    /*
     * Catalog stores and loads that pertain to this memory accessor/space
     * De-linearize step 0: get element size
     */
    Instruction *storeOrLoadUsingAccessor = nullptr;
    for (auto user : memoryAccessor->users()) {
      if (auto store = dyn_cast<StoreInst>(user)) {
        storeOrLoadUsingAccessor = store;
        accessSpaceByInstruction.insert(std::make_pair(store, memAccessSpace));
      } else if (auto load = dyn_cast<LoadInst>(user)) {
        storeOrLoadUsingAccessor = load;
        accessSpaceByInstruction.insert(std::make_pair(load, memAccessSpace));
      }
    }
    assert(storeOrLoadUsingAccessor != nullptr);
    memAccessSpace->memoryAccessorSCEV = SE.getSCEV(memAccessSpace->memoryAccessor);
    memAccessSpace->elementSize = SE.getElementSize(storeOrLoadUsingAccessor);

    // memAccessSpace->memoryAccessor->print(errs() << "Accessor: "); errs() << "\n";
    // memAccessSpace->memoryAccessorSCEV->print(errs() << "Accessor SCEV: "); errs() << "\n";

    if (!memAccessSpace->elementSize) continue;

    /*
     * De-linearize: collect parametric SCEV terms, dimension sizes, and computed access SCEVs per dimension
     */
    auto basePointer = dyn_cast<SCEVUnknown>(SE.getPointerBase(memAccessSpace->memoryAccessorSCEV));
    if (!basePointer) continue;

    auto accessFunction = SE.getMinusSCEV(memAccessSpace->memoryAccessorSCEV, basePointer);
    // TODO: Break apart usage of different steps in delinearization and modify the third
    // so that casts on subscripts is supported
    ScalarEvolutionDelinearization::delinearize(
      SE,
      accessFunction,
      memAccessSpace->subscripts,
      memAccessSpace->sizes,
      memAccessSpace->elementSize
    );

    if (memAccessSpace->subscripts.size() == 0) {
      if (auto gep = dyn_cast<GetElementPtrInst>(memAccessSpace->memoryAccessor)) {
        SmallVector<int, 4> sizes;
        // TODO: Determine exactly under what conditions size isn't returned from this API
        ScalarEvolutionDelinearization::getIndexExpressionsFromGEP(SE, gep, memAccessSpace->subscripts, sizes);
        for (auto size : sizes) {
          memAccessSpace->sizes.push_back(SE.getConstant(accessFunction->getType(), size));
        }
      }
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

void LoopIterationDomainSpaceAnalysis::identifyNonOverlappingAccessesBetweenIterationsAcrossOneLoopInvocation(
  ScalarEvolution &SE
) {

  for (auto &memAccessSpace : this->accessSpaces) {
    if (memAccessSpace->subscriptIVs.size() == 0) continue;

    // memAccessSpace->memoryAccessor->print(errs() << "Checking accessor for overlapping: "); errs() << "\n";

    /*
     * At least one dimension's subscript's IV must evolve in the top-most loop
     * This guarantees that for every iteration of the loop, the space accessed is unique
     */
    bool atLeastOneTopLevelIV = false;
    auto rootLoopStructure = loops.getLoopNestingTreeRoot();
    for (auto instIVPair : memAccessSpace->subscriptIVs) {
      auto iv = instIVPair.second;
      if (!iv) continue;

      auto loopEntryPHI = iv->getLoopEntryPHI();
      auto loopStructure = loops.getLoop(*loopEntryPHI);
      if (rootLoopStructure != loopStructure) continue;

      /*
       * The instruction pertaining to this IV must either be
       * 1) an instruction that matches the IV's evolution
       * 2) an instruction that derives from the IV through a one-to-one function to guarantee no overlap
       */
      auto inst = instIVPair.first;
      // inst->print(errs() << "Checking if I is non-overlapping root subscript: "); errs() << "\n";
      // SE.getSCEV(inst)->print(errs() << "SCEV for I: "); errs() << "\n";
      if (iv->isIVInstruction(inst) ||
        (iv->isDerivedFromIVInstructions(inst) &&
          isOneToOneFunctionOnIV(rootLoopStructure, iv, inst))) {
        atLeastOneTopLevelIV = true;
      }
    }
    if (!atLeastOneTopLevelIV) continue;

    nonOverlappingAccessesBetweenIterations.insert(memAccessSpace.get());
  }

  return;
}

void LoopIterationDomainSpaceAnalysis::identifyIVForMemoryAccessSubscripts (ScalarEvolution &SE) {

  auto findCorrespondingIVForSubscript = [&](
    const SCEV *subscriptSCEV
  ) -> std::pair<Instruction *, InductionVariable *> {

    /*
     * Constant subscripts are not linked to IVs
     */
    auto emptyPair = std::make_pair(nullptr, nullptr);
    if (isa<SCEVConstant>(subscriptSCEV)) return emptyPair;

    auto scevsMatch = [](const SCEV *scev1, const SCEV *scev2) -> bool {
      if (scev1 == scev2) return true;
      auto scevConstant1 = dyn_cast<SCEVConstant>(scev1);
      auto scevConstant2 = dyn_cast<SCEVConstant>(scev2);
      if (!scevConstant1 || !scevConstant2) return false;
      return scevConstant1->getValue()->getZExtValue() == scevConstant2->getValue()->getZExtValue();
    };

    /*
     * Identify an InductionVariable this SCEV belongs to by looking only within
     * the SCEV's loop
     * 
     * NOTE: This could result in one of several IVs being found if there exist
     * more than one evolving in lock-step
     */
    auto findInstructionInLoopForSCEV = [&SE, &scevsMatch](
      std::unordered_map<const SCEV *, std::unordered_set<Instruction *>> &scevToInstMap,
      const SCEV *subscriptSCEV
    ) -> Instruction * {
      if (scevToInstMap.find(subscriptSCEV) != scevToInstMap.end()) {
        auto matchingIVInstruction = *scevToInstMap.at(subscriptSCEV).begin();
        return matchingIVInstruction;
      }

      if (auto addRecSubscriptSCEV = dyn_cast<SCEVAddRecExpr>(subscriptSCEV)) {
        auto loopHeader = addRecSubscriptSCEV->getLoop()->getHeader();
        for (auto scevInstPair : scevToInstMap) {
          auto otherSCEV = scevInstPair.first;
          if (auto otherAddRecSCEV = dyn_cast<SCEVAddRecExpr>(otherSCEV)) {
            if (otherAddRecSCEV->getLoop()->getHeader() != loopHeader) continue;
            if (!scevsMatch(addRecSubscriptSCEV->getStart(), addRecSubscriptSCEV->getStart())) continue;
            if (!scevsMatch(addRecSubscriptSCEV->getStepRecurrence(SE), addRecSubscriptSCEV->getStepRecurrence(SE))) continue;
          }

          return *scevInstPair.second.begin();
        }
      }

      return nullptr;
    };

    // subscriptSCEV->print(errs() << "Searching for instruction matching subscript SCEV: "); errs() << "\n";
    auto ivInst = findInstructionInLoopForSCEV(this->ivInstructionsBySCEV, subscriptSCEV);
    if (ivInst) {
      // ivInst->print(errs() << "IV INST: "); errs() << "\n";
      auto iv = ivsByInstruction.at(ivInst);
      // iv->getLoopEntryPHI()->print(errs() << "IV LOOP ENTRY PHI: "); errs() << "\n";
      return std::make_pair(ivInst, iv);
    }

    auto derivedInst = findInstructionInLoopForSCEV(this->derivedInstructionsFromIVsBySCEV, subscriptSCEV);
    if (derivedInst) {
      // derivedInst->print(errs() << "DERIVED IV INST: "); errs() << "\n";
      auto derivingIV = ivsByInstruction.at(derivedInst);
      // derivingIV->getLoopEntryPHI()->print(errs() << "DERIVING IV LOOP ENTRY PHI: "); errs() << "\n";
      return std::make_pair(derivedInst, derivingIV);
    }

    return emptyPair;
  };

  for (auto &memAccessSpace : this->accessSpaces) {
    for (auto subscriptSCEV : memAccessSpace->subscripts) {
      auto ivOrNullptr = findCorrespondingIVForSubscript(subscriptSCEV);
      memAccessSpace->subscriptIVs.push_back(ivOrNullptr);
    }
  }

  return;
}

LoopIterationDomainSpaceAnalysis::MemoryAccessSpace::MemoryAccessSpace (Instruction *memoryAccessor)
  : memoryAccessor{memoryAccessor} {
}

LoopIterationDomainSpaceAnalysis::~LoopIterationDomainSpaceAnalysis () {
  accessSpaces.clear();
}

bool LoopIterationDomainSpaceAnalysis::isOneToOneFunctionOnIV(
  LoopStructure *loopStructure,
  InductionVariable *IV,
  Instruction *derivedInstruction
) {

  std::queue<Instruction *> derivingInsts;
  std::unordered_set<Instruction *> visited;
  derivingInsts.push(derivedInstruction);

  /*
   * Traverse all deriving operations from the derived instruction to instructions of the IV
   */
  while (!derivingInsts.empty()) {
    auto inst = derivingInsts.front();
    derivingInsts.pop();

    if (IV->isIVInstruction(inst)) continue;

    // inst->print(errs() << "Checking if derived inst is a one-to-one operation: "); errs() << "\n";

    /*
     * TODO: Make this far less naive
     */
    auto op = inst->getOpcode();
    bool isOneToOne = (op == Instruction::Add ||
      op == Instruction::Sub ||
      op == Instruction::Mul ||
      inst->isCast()
    );
    if (!isOneToOne) return false;

    for (auto &use : inst->operands()) {
      auto usedValue = use.get();

      /*
       * Ignore loop externals and constants as they are computed outside the loop
       * If the user can't be interpreted as an instruction, we do not know how to handle it 
       */
      if (isa<ConstantInt>(usedValue)) continue;
      auto usedInst = dyn_cast<Instruction>(usedValue);
      if (!usedInst) return false;
      if (!loopStructure->isIncluded(usedInst)) continue;

      if (visited.find(usedInst) != visited.end()) continue;
      visited.insert(usedInst);
      derivingInsts.push(usedInst);
    }
  }

  return true;
}