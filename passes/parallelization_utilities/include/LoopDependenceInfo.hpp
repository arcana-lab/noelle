/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "PDG.hpp"
#include "SCCDAG.hpp"
#include "LoopInfoSummary.hpp"
#include "SCCDAGAttrs.hpp"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Support/raw_ostream.h"
#include "LoopEnvironment.hpp"
#include "EnvBuilder.hpp"
#include "Techniques.hpp"

using namespace std;

namespace llvm {

  class LoopDependenceInfo {
    public:
      LoopInfoSummary liSummary;

      /*
       * Context
       */
      Function *function;

      /*
       * Loop entry and exit points.
       */
      BasicBlock *header;
      BasicBlock *preHeader;
      SmallVector<BasicBlock *, 10> loopExitBlocks;

      /*
       * Loop
       */
      std::vector<BasicBlock *> loopBBs;
      unordered_map<BasicBlock *, BasicBlock *> loopBBtoPD;

      /*
       * Environment
       */
      LoopEnvironment *environment;

      /*
       * Dependences
       */
      PDG *functionDG;
      PDG *loopDG;
      PDG *loopInternalDG;

      /*
       * SCCDAG.
       */
      // REFACTOR(angelo): rename loopSCCDAG to loopInternalSCCDAG
      SCCDAG *loopSCCDAG;
      SCCDAGAttrs sccdagAttrs;

      /*
       * Parallelization options
       */
      uint32_t maximumNumberOfCoresForTheParallelization;
      uint32_t DOALLChunkSize;

      /*
       * Methods
       */
      LoopDependenceInfo (
        Function *f,
        PDG *fG,
        Loop *l,
        LoopInfo &li,
        ScalarEvolution &SE
      );
      LoopDependenceInfo (
        Function *f,
        PDG *fG,
        Loop *l,
        LoopInfo &li,
        ScalarEvolution &SE,
        PostDominatorTree &pdt
      );

      void copyParallelizationOptionsFrom (LoopDependenceInfo *otherLDI) ;

      uint32_t numberOfExits (void) const;

      std::function<LoopDependenceInfo *(Function *F, int loopIndex)> *reevaluator;
      
      bool isTechniqueEnabled (Technique technique);

      void enableAllTechniques (void);

      void disableTechnique (Technique techniqueToDisable);

      ~LoopDependenceInfo();

    private:
      std::set<Technique> enabledTechniques;

      void fetchLoopAndBBInfo (LoopInfo &li, Loop *l) ;
      void createDGsForLoop (Loop *l) ;

      void mergeSingleSyntacticSugarInstrs ();
      void mergeBranchesWithoutOutgoingEdges ();
      void mergeTrivialNodesInSCCDAG ();
  };

}
