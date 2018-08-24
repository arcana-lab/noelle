#pragma once

#include "SCCDAG.hpp"
#include "LoopInfoSummary.hpp"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

namespace llvm {

  struct SCCEdgeInfo {
    std::set<Value *> edges;
  };

  enum class SCCExecutionType { Sequential, Associative, Independent };

  class SCCAttrs {
    public:

      /*
       * Fields
       */
      SCC *scc;
      std::set<BasicBlock *> bbs;
      int internalCost;
      bool hasLoopCarriedDataDep;
      bool isClonable;
      SCCExecutionType execType;
      std::unordered_map<SCC *, std::unique_ptr<SCCEdgeInfo>> sccToEdgeInfo;

      /*
       * Methods
       */
      SCCAttrs (SCC *s)
        : scc{s}, internalCost{0}, hasLoopCarriedDataDep{0}, isClonable{0} {
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
      std::set<SCC *> getSCCsWithLoopCarriedDataDependencies (void) const ;

      bool loopHasInductionVariable (ScalarEvolution &SE) const ;
      bool isInductionVariableSCC (ScalarEvolution &SE, SCC *scc) const ;
      bool isSCCContainedInSubloop (LoopInfoSummary &LIS, SCC *scc) const ;

      std::set<BasicBlock *> & getBasicBlocks (SCC *scc);

      int getSCCSubsetCost (std::set<SCC *> &sccs);

      // REFACTOR(angelo): find better workaround than just a getter for SCCAttrs
      std::unique_ptr<SCCAttrs> &getSCCAttrs (SCC *scc);

      void populate (SCCDAG *loopSCCDAG);

    private:
      std::unordered_map<SCC *, std::unique_ptr<SCCAttrs>> sccToInfo;
  };
}
