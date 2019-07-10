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

  class SCCAttrs {
    public:

      /*
       * Fields
       */
      SCC *scc;
      std::set<BasicBlock *> bbs;
      std::set<Value *> stronglyConnectedDataValues;
      std::set<Value *> weaklyConnectedDataValues;
      bool isClonable;
      bool hasIV;

      std::set<PHINode *> PHINodes;
      std::set<Instruction *> accumulators;
      PHINode *singlePHI;
      Instruction *singleAccumulator;
      std::set<TerminatorInst *> controlFlowInsts;
      std::set<std::pair<Value *, TerminatorInst *>> controlPairs;
      std::pair<Value *, TerminatorInst *> singleControlPair;

      /*
       * Methods
       */
      SCCAttrs (SCC *s);
      void collectSCCValues ();
  };

  //TODO: Have calculated by DOALL pass, not by SCCAttrs
  struct FixedIVBounds {
    Value *start;
    ConstantInt *step;
    Value *cmpIVTo;
    std::vector<Instruction *> cmpToDerivation;
    bool isCmpOnAccum;
    bool isCmpIVLHS;
    int endOffset;

    FixedIVBounds () : start{nullptr}, step{nullptr},
      cmpIVTo{nullptr}, endOffset{0}, cmpToDerivation{} {};
  };

  class SCCDAGAttrs {
    public:

      /*
       * Graph wide structures
       */
      SCCDAG *sccdag;
      AccumulatorOpInfo accumOpInfo;
      std::unordered_map<SCC *, std::unique_ptr<SCCAttrs>> sccToInfo;

      /*
       * Dependencies in graph
       */
      std::unordered_map<Value *, std::set<SCC *>> intraIterDeps;
      std::unordered_map<SCC *, std::set<DGEdge<Value> *>> interIterDeps;

      /*
       * Isolated clonable SCCs and resulting inherited parents
       */
      std::set<SCC *> clonableSCCs;
      std::unordered_map<SCC *, std::set<SCC *>> parentsViaClones;
      std::unordered_map<SCC *, std::set<DGEdge<SCC> *>> edgesViaClones;

      /*
       * Optional supplementary structures for some SCC
       */
      std::unordered_map<SCC *, FixedIVBounds *> sccIVBounds;

      /*
       * Methods on SCCDAG.
       */
      void populate (SCCDAG *loopSCCDAG, LoopInfoSummary &LIS, ScalarEvolution &SE);
      std::set<SCC *> getSCCsWithLoopCarriedDataDependencies (void) const ;
      bool isLoopGovernedByIV () const ;
      bool areAllLiveOutValuesReducable (LoopEnvironment *env) const ;

      /*
       * Methods on single SCC.
       */
      bool canExecuteReducibly (SCC *scc) const ;
      bool canExecuteIndependently (SCC *scc) const ;
      bool canBeCloned (SCC *scc) const ;
      bool isInductionVariableSCC (SCC *scc) const ;
      bool isSCCContainedInSubloop (LoopInfoSummary &LIS, SCC *scc) const ;
      std::set<BasicBlock *> & getBasicBlocks (SCC *scc);
      // REFACTOR(angelo): find better workaround than just a getter for SCCAttrs
      std::unique_ptr<SCCAttrs> &getSCCAttrs (SCC *scc); 

      /*
       * Methods about single dependence.
       */
      bool isALoopCarriedDependence (SCC *scc, DGEdge<Value> *dependence) ;

      /*
       * Methods about multiple dependences.
       */
      void iterateOverLoopCarriedDataDependences (
        SCC *scc, 
        std::function<bool (DGEdge<Value> *dependence)> func)
        ;

    private:

      /*
       * Helper methods on SCCDAG
       */
      void collectSCCGraphAssumingDistributedClones ();
      void collectDependencies (LoopInfoSummary &LIS);

      /*
       * Helper methods on single SCC
       */
      void collectPHIsAndAccumulators (SCC *scc);
      void collectControlFlowInstructions (SCC *scc);
      bool checkIfReducible (SCC *scc, LoopInfoSummary &LIS);
      bool checkIfIndependent (SCC *scc);
      bool checkIfInductionVariableSCC (SCC *scc, ScalarEvolution &SE, LoopInfoSummary &LIS);
      void checkIfIVHasFixedBounds (SCC *scc, LoopInfoSummary &LIS);
      bool isIVUpperBoundSimple (SCC *scc, FixedIVBounds &IVBounds, LoopInfoSummary &LIS);
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
      bool canPrecedeInCurrentIteration (LoopInfoSummary &LIS, Instruction *from, Instruction *to) const ;
  };
}
