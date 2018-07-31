#pragma once

#include "SCCDAG.hpp"
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
      bool hasLoopCarriedDep;
      std::unordered_map<SCC *, std::unique_ptr<SCCEdgeInfo>> sccToEdgeInfo;

      /*
       * Methods
       */
      SCCAttrs (SCC *s) : scc{s}, internalCost{0}, hasLoopCarriedDep{0} {
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
      bool doesHaveLoopCarriedDataDependences (void) const ;

      void setSCCToHaveLoopCarriedDataDependence (SCC *scc, bool doesItHaveLoopCarriedDataDependence);

      std::set<BasicBlock *> & getBasicBlocks (SCC *scc);

      int getSCCSubsetCost (std::set<SCC *> &sccs);

      // TODO(angelo): find better workaround than just a getter for SCCAttrs
      std::unique_ptr<SCCAttrs> &getSCCAttrs (SCC *scc);

      void populate (SCCDAG *loopSCCDAG);

    private:
      std::unordered_map<SCC *, std::unique_ptr<SCCAttrs>> sccToInfo;
  };
}
