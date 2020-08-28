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

#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "LoopsSummary.hpp"
#include "InductionVariables.hpp"
#include "Invariants.hpp"
#include "LoopGoverningIVAttribution.hpp"
#include "LoopIterationDomainSpaceAnalysis.hpp"
#include "SCCDAGAttrs.hpp"
#include "LoopEnvironment.hpp"
#include "EnvBuilder.hpp"
#include "Transformations.hpp"

#include "MemoryAnalysisModules/LoopAA.h"

namespace llvm {

  class LoopDependenceInfo {
    public:

      std::unordered_map<BasicBlock *, BasicBlock *> loopBBtoPD;  /*< From Basic block to its immediate post-dominatr.  */

      /*
       * Environment
       */
      LoopEnvironment *environment;

      /*
       * SCCDAG.
       */
      SCCDAGAttrs sccdagAttrs;

      /*
       * Parallelization options
       */
      uint32_t DOALLChunkSize;

      /*
       * Constructors.
       */
      LoopDependenceInfo (
        PDG *fG,
        Loop *l,
        DominatorSummary &DS,
        ScalarEvolution &SE,
        uint32_t maxCores
      );

      LoopDependenceInfo (
        PDG *fG,
        Loop *l,
        DominatorSummary &DS,
        ScalarEvolution &SE,
        uint32_t maxCores,
        liberty::LoopAA *aa
      );

      LoopDependenceInfo (
        PDG *fG,
        Loop *l,
        DominatorSummary &DS,
        ScalarEvolution &SE,
        uint32_t maxCores,
        std::unordered_set<LoopDependenceInfoOptimization> optimizations
      );

      LoopDependenceInfo (
        PDG *fG,
        Loop *l,
        DominatorSummary &DS,
        ScalarEvolution &SE,
        uint32_t maxCores,
        std::unordered_set<LoopDependenceInfoOptimization> optimizations,
        liberty::LoopAA *aa,
        bool enableLoopAwareDependenceAnalyses
      );

      LoopDependenceInfo () = delete ;

      /*
       * Return the ID of the loop.
       */
      uint64_t getID (void) const ;

      /*
       * Return the object containing all loop structures at and nested within this loop
       */
      const LoopsSummary & getLoopHierarchyStructures (void) const ;

      /*
       * Return the object that describes the loop in terms of induction variables, trip count, and control structure (e.g., latches, header)
       */
      LoopStructure * getLoopStructure (void) const ;

      /*
       * Return the nested-most loop for this instruction
       */
      LoopStructure * getNestedMostLoopStructure (Instruction *I) const ;

      /*
       * Get the dependence graph of the loop.
       */
      PDG * getLoopDG (void) const;

      /*
       * Copy all options from otherLDI to "this".
       */
      void copyParallelizationOptionsFrom (LoopDependenceInfo *otherLDI) ;

      /*
       * Return the number of exits of the loop.
       */
      uint32_t numberOfExits (void) const;

      /*
       * Check whether a transformation is enabled.
       */
      bool isTransformationEnabled (Transformation transformation);

      /*
       * Enable all transformations.
       */
      void enableAllTransformations (void);

      /*
       * Disable all transformations.
       */
      void disableTransformation (Transformation transformationToDisable);

      /*
       * Check whether an optimization is enabled
       */
      bool isOptimizationEnabled (LoopDependenceInfoOptimization optimization);

      /*
       * Iterate over children of "this" recursively following the loop nesting tree rooted by "this".
       * This will go through children of children etc...
       */
      bool iterateOverSubLoopsRecursively (
        std::function<bool (const LoopStructure &child)> funcToInvoke
        );

      /*
       * Return true if @param scc is fully contained in a subloop.
       * Return false otherwise.
       */
      bool isSCCContainedInSubloop (SCC *scc) const ;

      LoopGoverningIVAttribution * getLoopGoverningIVAttribution (void) const ;

      InductionVariableManager * getInductionVariableManager (void) const ;

      SCCDAGAttrs * getSCCManager (void) ;

      InvariantManager * getInvariantManager (void) const ;

      LoopIterationDomainSpaceAnalysis * getLoopIterationDomainSpaceAnalysis (void) const ;

      MemoryCloningAnalysis * getMemoryCloningAnalysis (void) const ;

      bool doesHaveCompileTimeKnownTripCount (void) const ;

      uint64_t getCompileTimeTripCount (void) const ;

      uint32_t getMaximumNumberOfCores (void) const ;

      /*
       * Deconstructor.
       */
      ~LoopDependenceInfo();

    private:

      /*
       * Fields
       */
      std::set<Transformation> enabledTransformations;  /* Transformations enabled. */
      std::unordered_set<LoopDependenceInfoOptimization> enabledOptimizations;  /* Optimizations enabled. */
      bool areLoopAwareAnalysesEnabled;

      PDG *loopDG;                            /* Dependence graph of the loop.
                                               * This graph does not include instructions outside the loop (i.e., no external dependences are included).
                                               */

      uint32_t maximumNumberOfCoresForTheParallelization;

      LoopsSummary liSummary;                 /* This field describes the loops with the current one as outermost.
                                               * Each loop is described in terms of its control structure (e.g., latches, header).
                                               */

      InductionVariableManager *inductionVariables;

      InvariantManager *invariantManager;

      LoopGoverningIVAttribution *loopGoverningIVAttribution;

      LoopIterationDomainSpaceAnalysis *domainSpaceAnalysis;

      MemoryCloningAnalysis *memoryCloningAnalysis;

      std::unordered_set<Value *> invariants;

      bool compileTimeKnownTripCount;

      uint64_t tripCount;

      /*
       * Methods
       */
      void fetchLoopAndBBInfo (
        Loop *l,
        ScalarEvolution &SE
        );

      std::pair<PDG *, SCCDAG *> createDGsForLoop (
        Loop *l,
        PDG *functionDG,
        DominatorSummary &DS,
        ScalarEvolution &SE,
        liberty::LoopAA *loopAA
        ) ;

      uint64_t computeTripCounts (
        Loop *l,
        ScalarEvolution &SE
        );

      void removeUnnecessaryDependenciesThatCloningMemoryNegates (
        PDG *loopInternalDG,
        DominatorSummary &DS,
        LoopCarriedDependencies &LCD
      ) ;

  };

}
