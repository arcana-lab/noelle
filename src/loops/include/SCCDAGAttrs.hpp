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
#include "LoopGoverningIVAttribution.hpp"
#include "LoopEnvironment.hpp"
#include "LoopCarriedDependencies.hpp"
#include "Variable.hpp"
#include "MemoryCloningAnalysis.hpp"

using namespace std;
using namespace llvm;

namespace llvm {

  /*
   * HACK: Remove once LoopDependenceInfo doesn't require argument-less SCCDAGAttrs constructor.
   */ 
  class LoopDependenceInfo;

  class SCCDAGAttrs {
    public:

      SCCDAGAttrs (
        PDG *loopDG,
        SCCDAG *loopSCCDAG,
        LoopsSummary &LIS,
        ScalarEvolution &SE,
        LoopCarriedDependencies &LCD,
        InductionVariableManager &IV,
        DominatorSummary &DS
      ) ;

      /*
       * Graph wide structures
       */
      AccumulatorOpInfo accumOpInfo;

      /*
       * Dependencies in graph
       */
      std::unordered_map<SCC *, Criticisms> sccToLoopCarriedDependencies;

      /*
       * Isolated clonable SCCs and resulting inherited parents
       */
      std::set<SCC *> clonableSCCs;
      std::unordered_map<SCC *, std::unordered_set<SCC *>> parentsViaClones;
      std::unordered_map<SCC *, std::unordered_set<DGEdge<SCC> *>> edgesViaClones;

      /*
       * Methods on SCCDAG.
       */
      std::set<SCC *> getSCCsWithLoopCarriedDependencies (void) const ;
      std::set<SCC *> getSCCsWithLoopCarriedDataDependencies (void) const ;
      std::set<SCC *> getSCCsWithLoopCarriedControlDependencies (void) const ;
      std::unordered_set<SCCAttrs *> getSCCsOfType (SCCAttrs::SCCType sccType);
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

      /*
       * Debug methods
       */
      void dumpToFile (int id) ;

      ~SCCDAGAttrs ();

    private:
      std::unordered_map<SCC *, SCCAttrs *> sccToInfo;
      PDG *loopDG;
      SCCDAG *sccdag;     /* SCCDAG of the related loop.  */
      MemoryCloningAnalysis *memoryCloningAnalysis;

      /*
       * HACK: Remove once LoopDependenceInfo doesn't produce empty SCCDAGAttrs on construction
       */
      friend class LoopDependenceInfo;
      SCCDAGAttrs () ;

      /*
       * Helper methods on SCCDAG
       */
      void collectSCCGraphAssumingDistributedClones ();
      void collectLoopCarriedDependencies (LoopsSummary &LIS, LoopCarriedDependencies &LCD);

      /*
       * Helper methods on single SCC
       */
      bool checkIfReducible (SCC *scc, LoopsSummary &LIS, LoopCarriedDependencies &LCD);
      bool checkIfIndependent (SCC *scc);
      bool checkIfSCCOnlyContainsInductionVariables (
        SCC *scc,
        LoopsSummary &LIS,
        std::set<InductionVariable *> &loopGoverningIVs,
        std::set<InductionVariable *> &IVs
      );
      void checkIfClonable (SCC *scc, ScalarEvolution &SE, LoopsSummary &LIS);
      void checkIfClonableByUsingLocalMemory(SCC *scc, LoopsSummary &LIS) ;
      bool isClonableByInductionVars (SCC *scc) const ;
      bool isClonableBySyntacticSugarInstrs (SCC *scc) const ;
      bool isClonableByCmpBrInstrs (SCC *scc) const ;
      bool isClonableByHavingNoMemoryOrLoopCarriedDataDependencies(SCC *scc, LoopsSummary &LIS) const ;

  };

}
