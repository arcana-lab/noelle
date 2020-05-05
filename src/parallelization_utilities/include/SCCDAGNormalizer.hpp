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
#include "LoopsSummary.hpp"
#include "DominatorSummary.hpp"
#include "SCCDAG.hpp"
#include "SCCDAGAttrs.hpp"

namespace llvm {

  class SCCDAGNormalizer {
    public:

      SCCDAGNormalizer(SCCDAG &dag, LoopsSummary &lis, ScalarEvolution &se, DominatorSummary &ds)
        : LIS{lis}, SE{se}, DS{ds}, sccdag{dag} {}

      void normalizeInPlace() ;

    private:
      LoopsSummary &LIS;
      ScalarEvolution &SE;
      DominatorSummary &DS;
      SCCDAG &sccdag;

      void mergeLCSSAPhis () ;
      void mergeSCCsWithExternalInterIterationDependencies () ;
      void mergeSingleSyntacticSugarInstrs () ;
      void mergeBranchesWithoutOutgoingEdges () ;
    
      class MergeGroups {
        public:
          std::unordered_map<DGNode<SCC> *, std::set<DGNode<SCC> *> *> sccToGroupMap;
          std::set<std::set<DGNode<SCC> *> *> groups;

          MergeGroups() : sccToGroupMap{} {}
          ~MergeGroups() ;

          void merge(DGNode<SCC> *sccNode1, DGNode<SCC> *sccNode2) ;
      };
  };
}