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
#ifndef NOELLE_SRC_TOOLS_SCEV_SIMPLIFICATION_SCEVSIMPLIFICATION_H_
#define NOELLE_SRC_TOOLS_SCEV_SIMPLIFICATION_SCEVSIMPLIFICATION_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/Noelle.hpp"
#include "arcana/noelle/core/LoopContent.hpp"

namespace arcana::noelle {

class SCEVSimplification {
public:
  SCEVSimplification(Noelle &noelle);

  bool simplifyIVRelatedSCEVs(LoopContent const &LDI);

  bool simplifyIVRelatedSCEVs(LoopTree *rootLoopNode,
                              InvariantManager *invariantManager,
                              InductionVariableManager *ivManager);

  bool simplifyLoopGoverningIVGuards(LoopContent const &LDI,
                                     ScalarEvolution &SE);

  bool simplifyConstantPHIs(LoopContent const &LDI);

private:
  const SCEV *getOffsetBetween(ScalarEvolution &SE,
                               const SCEV *startSCEV,
                               const SCEV *intermediateSCEV);

  /*
   * IV cache for quick IV lookup
   */
  struct IVCachedInfo {
    std::unordered_map<Instruction *, InductionVariable *> ivByInstruction;
    std::unordered_map<InductionVariable *, LoopGoverningInductionVariable *>
        loopGoverningAttrByIV;
    std::unordered_set<Instruction *> instsDerivedFromMultipleIVs;
  };

  class GEPIndexDerivation {
  public:
    GEPIndexDerivation(GetElementPtrInst *gep,
                       LoopStructure *rootLoop,
                       InvariantManager *invariantManager,
                       IVCachedInfo &ivCache);

    bool isDerivingInstruction(Value *value) const;

    GetElementPtrInst *gep;
    bool isDerived;

    std::unordered_set<Value *> loopInvariantsUsed;
    std::unordered_set<InductionVariable *> derivingIVs;
    std::unordered_set<Instruction *> ivDerivingInstructions;
  };

  void cacheIVInfo(IVCachedInfo &ivCache,
                   LoopTree *rootLoopNode,
                   InductionVariableManager *ivManager);

  void searchForInstructionsDerivedFromMultipleIVs(
      IVCachedInfo &ivCache,
      LoopStructure *rootLoop,
      InvariantManager *invariantManager);

  bool upCastIVRelatedInstructionsDerivingGEP(
      IVCachedInfo &ivCache,
      LoopTree *rootLoopNode,
      InductionVariableManager *ivManager,
      InvariantManager *invariantManager,
      std::unordered_set<GEPIndexDerivation *> gepDerivations);

  bool isUpCastPossible(GEPIndexDerivation *gepDerivation,
                        LoopStructure *rootLoop,
                        InvariantManager &invariantManager) const;
  bool isPartOfShlShrTruncationPair(Instruction *I) const;

  /*
   * Fields
   */
  Noelle &noelle;
  unsigned ptrSizeInBits;
  IntegerType *intTypeForPtrSize;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_TOOLS_SCEV_SIMPLIFICATION_SCEVSIMPLIFICATION_H_
