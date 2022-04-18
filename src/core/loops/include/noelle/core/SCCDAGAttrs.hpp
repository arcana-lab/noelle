  /*
   * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni, Brian Homerding
   *
   * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

   * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

   * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
   * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
   */
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/AccumulatorOpInfo.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/SCCAttrs.hpp"
#include "noelle/core/InductionVariables.hpp"
#include "noelle/core/LoopGoverningIVAttribution.hpp"
#include "noelle/core/LoopEnvironment.hpp"
#include "noelle/core/Variable.hpp"
#include "noelle/core/MemoryCloningAnalysis.hpp"

namespace llvm::noelle {

  class SCCDAGAttrs {
    public:

      SCCDAGAttrs (
        bool enableFloatAsReal,
        PDG *loopDG,
        SCCDAG *loopSCCDAG,
        StayConnectedNestedLoopForestNode *loopNode,
        ScalarEvolution &SE,
        InductionVariableManager &IV,
        DominatorSummary &DS
      ) ;
      
      SCCDAGAttrs () = delete ;

      /*
       * Graph wide structures
       */
      AccumulatorOpInfo accumOpInfo;

      /*
       * Dependencies in graph
       */
      std::map<SCC *, Criticisms> sccToLoopCarriedDependencies;

      /*
       * Isolated clonable SCCs and resulting inherited parents
       */
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
      bool isSCCContainedInSubloop (
        StayConnectedNestedLoopForestNode *loop,
        SCC *scc
        ) const ;
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
        std::function<bool (DGEdge<Value> *dependence)> func
        ) ;

      void iterateOverLoopCarriedControlDependences (
        SCC *scc, 
        std::function<bool (DGEdge<Value> *dependence)> func
        ) ;

      void iterateOverLoopCarriedDependences (
        SCC *scc, 
        std::function<bool (DGEdge<Value> *dependence)> func
        ) ;

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
      bool enableFloatAsReal;
      std::unordered_map<SCC *, SCCAttrs *> sccToInfo;
      PDG *loopDG;
      SCCDAG *sccdag;     /* SCCDAG of the related loop.  */
      MemoryCloningAnalysis *memoryCloningAnalysis;

      /*
       * Helper methods on SCCDAG
       */
      void collectSCCGraphAssumingDistributedClones ();
      void collectLoopCarriedDependencies (StayConnectedNestedLoopForestNode *loopNode);

      /*
       * Helper methods on single SCC
       */
      bool checkIfReducible (
        SCC *scc,
        StayConnectedNestedLoopForestNode *loop
        );
      bool checkIfIndependent (SCC *scc);
      bool checkIfSCCOnlyContainsInductionVariables (
        SCC *scc,
        StayConnectedNestedLoopForestNode *loop,
        std::set<InductionVariable *> &loopGoverningIVs,
        std::set<InductionVariable *> &IVs
      );
      void checkIfClonable (
        SCC *scc, 
        ScalarEvolution &SE,
        StayConnectedNestedLoopForestNode *loop
        );
      void checkIfClonableByUsingLocalMemory (
        SCC *scc,
        StayConnectedNestedLoopForestNode *loop
        );
      bool isClonableByInductionVars (SCC *scc) const ;
      bool isClonableBySyntacticSugarInstrs (SCC *scc) const ;
      bool isClonableByCmpBrInstrs (SCC *scc) const ;
      bool isClonableByHavingNoMemoryOrLoopCarriedDataDependencies (
        SCC *scc,
        StayConnectedNestedLoopForestNode *loop
        ) const ;
  };

}
