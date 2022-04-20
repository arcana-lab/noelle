/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni, Brian Homerding
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "noelle/core/LoopStructure.hpp"
#include "noelle/core/SCC.hpp"
#include "noelle/core/SCCDAG.hpp"
#include "noelle/core/DGGraphTraits.hpp"
#include "noelle/core/StayConnectedNestedLoopForest.hpp"

namespace llvm {
  namespace noelle {

    struct SCCSet {
      std::unordered_set<SCC *> sccs;

      raw_ostream &print (raw_ostream &stream) ;
    };

    class SCCDAGPartition : public DG<SCCSet> {
      public:

        /*
        * sccToParentsMap: A custom relation mapping that allows certain SCC to be ignored during
        * partitioning (i.e. SCC which are not to be partitioned such as clonable SCC simply aren't
        * mentioned in this relation and are not given a node in this graph)
        */
        SCCDAGPartition (
          SCCDAG *sccdag,
          std::unordered_set<SCCSet *> initialSets,
          std::unordered_map<SCC *, std::unordered_set<SCC *>> sccToParentsMap
        ) ;

        ~SCCDAGPartition () ;

        SCC *sccOfValue (Value *V) ;

        bool isIncludedInPartitioning (SCC *scc) ;

        SCCSet *setOfSCC (SCC *scc) ;

        void mergeSetsAndCollapseResultingCycles (std::unordered_set<SCCSet *> sets) ;

        std::vector<SCCSet *> getDepthOrderedSets (void) ;

        SCCDAG *getSCCDAG (void) const ;

      private:

        void mergeSets (std::unordered_set<SCCSet *> sets) ;
        void collapseCycles (void) ;

        /*
        * The SCCDAG being partitioned
        */
        SCCDAG *sccdag;

        /*
        * A mapping from SCC to its set in the partitioning
        */
        std::unordered_map<SCC *, SCCSet *> sccToSetMap;

    };

    class SCCDAGPartitioner {
      public:
        SCCDAGPartitioner (
          SCCDAG *sccdag,
          std::unordered_set<SCCSet *> initialSets,
          std::unordered_map<SCC *, std::unordered_set<SCC *>> sccToParentsMap,
          StayConnectedNestedLoopForestNode *loopNode
        );

        SCCDAGPartitioner () = delete ;

        ~SCCDAGPartitioner () ;

        uint64_t numberOfPartitions (void);

        SCCDAGPartition *getPartitionGraph (void) ;
        std::unordered_set<SCCSet *> getParents (SCCSet *set) ;
        std::unordered_set<SCCSet *> getChildren (SCCSet *set) ;
        std::unordered_set<SCCSet *> getSets (void) ;
        std::unordered_set<SCCSet *> getRoots (void) ;
        std::vector<SCCSet *> getDepthOrderedSets (void) ;

        bool isMergeIntroducingCycle (SCCSet *setA, SCCSet *setB) ;

        std::unordered_set<SCCSet *> getCycleIntroducedByMerging (SCCSet *setA, SCCSet *setB) ;

        bool isAncestor (SCCSet *parentTarget, SCCSet *target) ;

        std::pair<SCCSet *, SCCSet *> getParentChildPair (SCCSet *setA, SCCSet *setB) ;

        std::unordered_set<SCCSet *> getDescendants (SCCSet *set) ;
        std::unordered_set<SCCSet *> getAncestors (SCCSet *set) ;
        std::unordered_set<SCCSet *> getOverlap(std::unordered_set<SCCSet *> setsA, std::unordered_set<SCCSet *> setsB) ;

        SCCSet *mergePair (SCCSet *setA, SCCSet *setB) ;

//      void mergeLoopCarriedDependencies (LoopCarriedDependencies *LCD) ;

        void mergeLCSSAPhisWithTheValuesTheyPropagate (void) ;

        void mergeAlongMemoryEdges (void) ;

        raw_ostream &printSet (raw_ostream &stream, SCCSet *set) ;

        // raw_ostream &print (raw_ostream &stream, std::string prefix) ;
        // std::string subsetStr (SCCset *subset);
        // raw_ostream &printSCCIndices (raw_ostream &stream, std::string prefix);
        // raw_ostream &printNodeInGraph (raw_ostream &stream, std::string prefix, SCCset *subset);
        // raw_ostream &printGraph (raw_ostream &stream, std::string prefix);

      private:

        void resetPartitioner (void);

        void mergeAllPairs (std::set<std::pair<SCC *, SCC *>> pairs) ;

        /*
        * Debug information at the SCC level
        */
        std::vector<SCC *> SCCDebugOrder;
        std::unordered_map<SCC *, int> SCCDebugIndex;

        SCCDAGPartition *partition;
        StayConnectedNestedLoopForestNode *rootLoop;
        std::unordered_set<StayConnectedNestedLoopForestNode *> allLoops;
    };

  }

  template<> struct GraphTraits<DGGraphWrapper<llvm::noelle::SCCDAGPartition, llvm::noelle::SCCSet> *> : 
    public GraphTraitsBase<
      DGGraphWrapper<llvm::noelle::SCCDAGPartition, llvm::noelle::SCCSet>,
      DGNodeWrapper<llvm::noelle::SCCSet>,
      llvm::noelle::SCCSet
    > {};

}
