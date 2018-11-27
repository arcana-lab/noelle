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
       * TODO(angelo): allow multiple phis as long as they form a single cycle
       */
      PHINode * singlePHI;
      std::set<Instruction *> PHIAccumulators;

      bool isSimpleIV;
      SimpleIVInfo simpleIVInfo;

      /*
       * Methods
       */
      SCCAttrs (SCC *s)
        : scc{s}, isIndependent{0}, isClonable{0},
          isReducable{0}, isSimpleIV{0}, singlePHI{nullptr} {
        // Collect basic blocks contained within SCC
        for (auto nodePair : this->scc->internalNodePairs()) {
          this->bbs.insert(cast<Instruction>(nodePair.first)->getParent());
        }
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
      void collectSinglePHIAndAccumulators (SCC *scc);
      bool checkIfCommutative (SCC *scc);
      bool checkIfIndependent (SCC *scc);
      bool checkIfInductionVariableSCC (SCC *scc, ScalarEvolution &SE);
      bool checkIfSimpleIV (SCC *scc, LoopInfoSummary &LIS);
      bool checkSimpleIVEndVal (SimpleIVInfo &ivInfo, LoopInfoSummary &LIS);
      void checkIfClonable (SCC *scc, ScalarEvolution &SE);
      bool isClonableByInductionVars (SCC *scc) const ;
      bool isClonableBySyntacticSugarInstrs (SCC *scc) const ;
      bool isClonableByCmpBrInstrs (SCC *scc) const ;
  };
}
