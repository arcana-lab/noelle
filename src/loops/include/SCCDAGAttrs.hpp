/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"
#include "AccumulatorOpInfo.hpp"
#include "SCCDAG.hpp"
#include "SCC.hpp"
#include "SCCAttrs.hpp"
#include "LoopsSummary.hpp"
#include "InductionVariables.hpp"
#include "LoopEnvironment.hpp"
#include "DominatorSummary.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

  class SCCDAGAttrs {
    public:

      /*
       * Graph wide structures
       */
      AccumulatorOpInfo accumOpInfo;

      /*
       * Dependencies in graph
       */
      std::unordered_map<SCC *, std::set<DGEdge<Value> *>> intraIterDeps;
      std::unordered_map<SCC *, std::set<DGEdge<Value> *>> interIterDeps;
      std::unordered_map<SCC *, std::set<DGEdge<Value> *>> interIterDepsInternalToSCC;

      /*
       * Isolated clonable SCCs and resulting inherited parents
       */
      std::set<SCC *> clonableSCCs;
      std::unordered_map<SCC *, std::set<SCC *>> parentsViaClones;
      std::unordered_map<SCC *, std::set<DGEdge<SCC> *>> edgesViaClones;

      /*
       * Constructors.
       */
      void populate (SCCDAG *loopSCCDAG, LoopsSummary &LIS, ScalarEvolution &SE, DominatorSummary &DS, InductionVariables &IV);

      /*
       * Methods on SCCDAG.
       */
      std::set<SCC *> getSCCsWithLoopCarriedDependencies (void) const ;
      std::set<SCC *> getSCCsWithLoopCarriedDataDependencies (void) const ;
      std::set<SCC *> getSCCsWithLoopCarriedControlDependencies (void) const ;
      bool isLoopGovernedBySCC (SCC *scc) const ;
      bool areAllLiveOutValuesReducable (LoopEnvironment *env) const ;

      /*
       * Methods on single SCC.
       */
      bool isSCCContainedInSubloop (const LoopsSummary &LIS, SCC *scc) const ;
      SCCAttrs * getSCCAttrs (SCC *scc) const; 

      /*
       * Methods about single dependence.
       */
      bool isALoopCarriedDependence (SCC *scc, DGEdge<Value> *dependence) ;

      /*
       * Methods about multiple dependences.
       */
      void iterateOverLoopCarriedDataDependences (
        SCC *scc, 
        std::function<bool (DGEdge<Value> *dependence)> func) ;

      /*
       * Return the SCCDAG of the loop.
       */
      // TODO: Return const reference to SCCDAG, not a raw pointer
      SCCDAG * getSCCDAG (void) const ;

    private:
      std::unordered_map<SCC *, SCCAttrs *> sccToInfo;
      SCCDAG *sccdag;     /* SCCDAG of the related loop.  */

      /*
       * Helper methods on SCCDAG
       */
      void collectSCCGraphAssumingDistributedClones ();
      void collectDependencies (LoopsSummary &LIS, DominatorSummary &DS);

      /*
       * Helper methods on single SCC
       */
      bool checkIfReducible (SCC *scc, LoopsSummary &LIS);
      bool checkIfIndependent (SCC *scc);
      bool checkIfSCCOnlyContainsInductionVariables (
        SCC *scc,
        LoopsSummary &LIS,
        std::set<InductionVariable *> &loopGoverningIVs,
        std::set<InductionVariable *> &IVs
      );
      void checkIfClonable (SCC *scc, ScalarEvolution &SE);
      bool isClonableByInductionVars (SCC *scc) const ;
      bool isClonableBySyntacticSugarInstrs (SCC *scc) const ;
      bool isClonableByCmpBrInstrs (SCC *scc) const ;

      /*
       * Helper methods on single values within SCCs
       */
      bool isDerivedWithinSCC (Value *V, SCC *scc) const ;
      bool isDerivedPHIOrAccumulator (Value *V, SCC *scc) const ;
      bool collectDerivationChain (std::vector<Instruction *> &chain, SCC *scc);
  };

}
