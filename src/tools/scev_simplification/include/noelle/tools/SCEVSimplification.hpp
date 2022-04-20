/*
 * Copyright 2016 - 2020  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/Noelle.hpp"
#include "noelle/core/LoopDependenceInfo.hpp"

namespace llvm::noelle {

  class SCEVSimplification {
    public:
      SCEVSimplification (Noelle &noelle);

      bool simplifyIVRelatedSCEVs (
        LoopDependenceInfo const &LDI
      );

      bool simplifyIVRelatedSCEVs (
        StayConnectedNestedLoopForestNode *rootLoopNode, 
        InvariantManager *invariantManager,
        InductionVariableManager *ivManager
      );

      bool simplifyLoopGoverningIVGuards (
        LoopDependenceInfo const &LDI,
        ScalarEvolution &SE
      );

      bool simplifyConstantPHIs (
        LoopDependenceInfo const &LDI
      );

    private:

      const SCEV *getOffsetBetween (ScalarEvolution &SE, const SCEV *startSCEV, const SCEV *intermediateSCEV) ;

      /*
       * IV cache for quick IV lookup
       */
      struct IVCachedInfo {
        std::unordered_map<Instruction *, InductionVariable *> ivByInstruction;
        std::unordered_map<InductionVariable *, LoopGoverningIVAttribution *> loopGoverningAttrByIV;
        std::unordered_set<Instruction *> instsDerivedFromMultipleIVs;
      };

      class GEPIndexDerivation {
        public:
          GEPIndexDerivation (
            GetElementPtrInst *gep,
            LoopStructure *rootLoop,
            InvariantManager *invariantManager,
            IVCachedInfo &ivCache
            ) ;

          bool isDerivingInstruction (Value *value) const ;

          GetElementPtrInst *gep;
          bool isDerived;

          std::unordered_set<Value *> loopInvariantsUsed;
          std::unordered_set<InductionVariable *> derivingIVs;
          std::unordered_set<Instruction *> ivDerivingInstructions;

      };

      void cacheIVInfo (
        IVCachedInfo &ivCache,
        StayConnectedNestedLoopForestNode *rootLoopNode,
        InductionVariableManager *ivManager
      ) ;

      void searchForInstructionsDerivedFromMultipleIVs (
        IVCachedInfo &ivCache,
        LoopStructure *rootLoop,
        InvariantManager *invariantManager
      ) ;

      bool upCastIVRelatedInstructionsDerivingGEP (
        IVCachedInfo &ivCache,
        StayConnectedNestedLoopForestNode *rootLoopNode, 
        InductionVariableManager *ivManager,
        InvariantManager *invariantManager,
        std::unordered_set<GEPIndexDerivation *> gepDerivations
      );

      bool isUpCastPossible (
        GEPIndexDerivation *gepDerivation,
        LoopStructure *rootLoop,
        InvariantManager &invariantManager
      ) const ;
      bool isPartOfShlShrTruncationPair (Instruction *I) const ;


      /*
       * Fields
       */
      Noelle &noelle;
      unsigned ptrSizeInBits;
      IntegerType *intTypeForPtrSize;

  };

}
