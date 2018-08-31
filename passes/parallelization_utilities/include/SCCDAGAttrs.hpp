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

  struct SCCEdgeInfo {
    std::set<Value *> edges;
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
      std::unordered_map<SCC *, std::unique_ptr<SCCEdgeInfo>> sccToEdgeInfo;

      /*
       * Methods
       */
      SCCAttrs (SCC *s)
        : scc{s}, internalCost{0}, isIndependent{0},
          isClonable{0}, isReducable{0} {
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

      /*
       * Methods
       */
      bool executesCommutatively (SCC *scc);
      bool executesIndependently (SCC *scc);
      bool canBeCloned (SCC *scc);
      std::set<SCC *> getSCCsWithLoopCarriedDataDependencies (void) const ;

      bool loopHasInductionVariable (ScalarEvolution &SE) const ;
      bool isInductionVariableSCC (ScalarEvolution &SE, SCC *scc) const ;
      bool isSCCContainedInSubloop (LoopInfoSummary &LIS, SCC *scc) const ;
      bool allPostLoopEnvValuesAreReducable (LoopEnvironment *env) const ;

      std::set<BasicBlock *> & getBasicBlocks (SCC *scc);

      int getSCCSubsetCost (std::set<SCC *> &sccs);

      // REFACTOR(angelo): find better workaround than just a getter for SCCAttrs
      std::unique_ptr<SCCAttrs> &getSCCAttrs (SCC *scc);

      void populate (SCCDAG *loopSCCDAG, ScalarEvolution &SE);

    private:
      bool checkIfCommutative (SCC *scc);
      bool checkIfIndependent (SCC *scc);
      void checkIfClonable (SCC *scc, ScalarEvolution &SE);
      void checkIfClonableByInductionVars (SCC *scc, ScalarEvolution &SE);
      void checkIfClonableBySyntacticSugarInstrs (SCC *scc);

      std::unordered_map<SCC *, std::unique_ptr<SCCAttrs>> sccToInfo;
  };
}
