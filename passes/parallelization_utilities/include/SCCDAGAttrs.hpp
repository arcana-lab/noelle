/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SCCDAG.hpp"
#include "SCC.hpp"
#include "LoopInfoSummary.hpp"
#include "LoopEnvironment.hpp"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

namespace llvm {

  struct AccumulatorOpInfo {
    AccumulatorOpInfo ();

    std::set<unsigned> sideEffectFreeOps;
    std::set<unsigned> accumOps;
    std::unordered_map<unsigned, unsigned> opIdentities;

    bool isMulOp (unsigned op);
    bool isAddOp (unsigned op);
    bool isSubOp (unsigned op);
    unsigned accumOpForType (unsigned op, Type *type);
    Value *generateIdentityFor (Instruction *accumulator, Type *castType);
  };

  /*
   * TODO(angelo): Fully understand SCEV to render this characterization
   * of an IV obsolete
   */
  struct SimpleIVInfo {
    CmpInst *cmp;
    BranchInst *br;
    Value *start;
    ConstantInt *step;
    Value *cmpIVTo;
    bool isCmpOnAccum;
    bool isCmpIVLHS;
    int endOffset;

    SimpleIVInfo () : cmp{nullptr}, br{nullptr}, start{nullptr},
      step{nullptr}, cmpIVTo{nullptr}, endOffset{0} {};
  };

  class SCCAttrs {
    public:

      /*
       * Fields
       */
      SCC *scc;
      std::set<BasicBlock *> bbs;
      bool isIndependent;
      bool isClonable;
      bool isReducable;
      bool isIVSCC;

      /*
       * Fields used if the SCC is of a "simple" form,
       * consisting of only PHIs, binary accumulators,
       * control flow (compares / branches), and cast instructions
       */
      std::set<PHINode *> PHINodes;
      std::set<Instruction *> accumulators;
      PHINode *singlePHI;
      Instruction *singleAccumulator;
      SimpleIVInfo *simpleIVInfo;

      /*
       * Methods
       */
      SCCAttrs (SCC *s)
        : scc{s}, isIndependent{0}, isClonable{0},
          isReducable{0}, singlePHI{nullptr},
          singleAccumulator{nullptr}, simpleIVInfo{nullptr},
          PHINodes{}, accumulators{} {
        // Collect basic blocks contained within SCC
        for (auto nodePair : this->scc->internalNodePairs()) {
          this->bbs.insert(cast<Instruction>(nodePair.first)->getParent());
        }
      }
      ~SCCAttrs () {
        if (simpleIVInfo) delete simpleIVInfo;
      }
  };

  class SCCDAGAttrs {
    public:

      /*
       * Fields
       */
      SCCDAG *sccdag;
      std::unordered_map<SCC *, std::unique_ptr<SCCAttrs>> sccToInfo;
      AccumulatorOpInfo accumOpInfo;
      std::set<SCC *> clonableSCCs;
      std::unordered_map<SCC *, std::set<SCC *>> parentsViaClones;
      std::unordered_map<SCC *, std::set<DGEdge<SCC> *>> edgesViaClones;

      /*
       * Methods on SCCDAG.
       */
      void populate (SCCDAG *loopSCCDAG, LoopInfoSummary &LIS, ScalarEvolution &SE);
      std::set<SCC *> getSCCsWithLoopCarriedDataDependencies (void) const ;
      bool doesLoopHaveIV () const ;
      bool areAllLiveOutValuesReducable (LoopEnvironment *env) const ;

      /*
       * Methods on single SCC.
       */
      bool canExecuteCommutatively (SCC *scc) const ;
      bool canExecuteIndependently (SCC *scc) const ;
      bool canBeCloned (SCC *scc) const ;
      bool isInductionVariableSCC (SCC *scc) const ;
      bool isSCCContainedInSubloop (LoopInfoSummary &LIS, SCC *scc) const ;
      std::set<BasicBlock *> & getBasicBlocks (SCC *scc);
      // REFACTOR(angelo): find better workaround than just a getter for SCCAttrs
      std::unique_ptr<SCCAttrs> &getSCCAttrs (SCC *scc); 

    private:
      /*
       * Helper methods on SCCDAG
       */
      void collectSCCGraphAssumingDistributedClones ();

      /*
       * Helper methods on single SCC
       */
      void collectPHIsAndAccumulators (SCC *scc);
      bool checkIfCommutative (SCC *scc);
      bool checkIfIndependent (SCC *scc);
      bool checkIfInductionVariableSCC (SCC *scc, ScalarEvolution &SE);
      void checkIfSimpleIV (SCC *scc, LoopInfoSummary &LIS);
      bool doesIVHaveSimpleEndVal (SimpleIVInfo &ivInfo, LoopInfoSummary &LIS);
      void checkIfClonable (SCC *scc, ScalarEvolution &SE);
      bool isClonableByInductionVars (SCC *scc) const ;
      bool isClonableBySyntacticSugarInstrs (SCC *scc) const ;
      bool isClonableByCmpBrInstrs (SCC *scc) const ;
  };
}
