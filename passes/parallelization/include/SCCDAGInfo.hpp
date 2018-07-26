#pragma once

#include "SCCDAG.hpp"
#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace llvm;

namespace llvm {

  /*
   * Modelling assumption: Each edge listed may yield potential queue of unit cost
   */
  struct SCCEdgeInfo {
    std::set<Value *> edges;
  };

  class SCCInfo {
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
      SCCInfo (SCC *s) : scc{s}, internalCost{0}, hasLoopCarriedDep{0} {
        for (auto nodePair : this->scc->internalNodePairs()) {
          this->bbs.insert(cast<Instruction>(nodePair.first)->getParent());
        }
      }
  };

  class SCCDAGInfo {
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

      void populate (SCCDAG *loopSCCDAG);

    //private: TODO to add asap
      std::unordered_map<SCC *, std::unique_ptr<SCCInfo>> sccToInfo;
  };
}
