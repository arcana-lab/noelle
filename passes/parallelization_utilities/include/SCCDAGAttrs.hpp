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
  };

  struct SCCEdgeInfo {
    std::set<Value *> edges;
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
      int internalCost;
      bool isIndependent;
      bool isClonable;
      bool isReducable;
      bool isIVSCC;
      std::unordered_map<SCC *, std::unique_ptr<SCCEdgeInfo>> sccToEdgeInfo;

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
        : scc{s}, internalCost{0}, isIndependent{0}, isClonable{0},
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
      AccumulatorOpInfo accumOpInfo;
      std::set<SCC *> clonableSCCs;

      /*
       * Methods
       */
      bool executesCommutatively (SCC *scc);
      bool executesIndependently (SCC *scc);
      bool canBeCloned (SCC *scc);
      bool isInductionVariableSCC (SCC *scc);
      std::set<SCC *> getSCCsWithLoopCarriedDataDependencies (void) const ;

      bool loopHasInductionVariable ();
      bool isSCCContainedInSubloop (LoopInfoSummary &LIS, SCC *scc) const ;
      bool allPostLoopEnvValuesAreReducable (LoopEnvironment *env) const ;

      std::set<BasicBlock *> & getBasicBlocks (SCC *scc);

      int getSCCSubsetCost (std::set<SCC *> &sccs);

      // REFACTOR(angelo): find better workaround than just a getter for SCCAttrs
      std::unique_ptr<SCCAttrs> &getSCCAttrs (SCC *scc);

      void populate (SCCDAG *loopSCCDAG, LoopInfoSummary &LIS, ScalarEvolution &SE);

    private:
      void collectSinglePHIAndAccumulators (SCC *scc);
      bool checkIfCommutative (SCC *scc);
      bool checkIfIndependent (SCC *scc);
      bool checkIfInductionVariableSCC (SCC *scc, ScalarEvolution &SE);
      bool checkIfSimpleIV (SCC *scc, LoopInfoSummary &LIS);
      bool checkSimpleIVEndVal (SimpleIVInfo &ivInfo, LoopInfoSummary &LIS);
      void checkIfClonable (SCC *scc, ScalarEvolution &SE);
      bool checkIfClonableByInductionVars (SCC *scc);
      bool checkIfClonableBySyntacticSugarInstrs (SCC *scc);

      std::unordered_map<SCC *, std::unique_ptr<SCCAttrs>> sccToInfo;
  };
}
