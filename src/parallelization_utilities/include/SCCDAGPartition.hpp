/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SCC.hpp"
#include "SCCDAG.hpp"
#include "SCCDAGAttrs.hpp"
#include "LoopsSummary.hpp"

typedef typename std::set<SCC *> SCCset;

class SCCDAGPartition {
  public:
    SCCDAGPartition (
      SCCDAG *dag,
      SCCDAGAttrs *attrs,
      LoopsSummary *LIS,
      std::set<SCCset *> *sets
    );

    void resetPartition (std::set<SCCset *> *subsets);

    SCCset *mergePairAndCycles (SCCset *subsetA, SCCset *subsetB);

    SCCset *mergePair (SCCset *subsetA, SCCset *subsetB, bool doReorder = true);

    bool mergeYieldsCycle (SCCset *subsetA, SCCset *subsetB);

    bool mergeAlongMemoryEdges ();

    uint64_t numberOfPartitions (void);

    std::set<SCCset *> *getSubsets() { return subsets; }
    std::set<SCCset *> *getRoots() { return &roots; }
    std::set<SCCset *> *getParents(SCCset *subset);
    std::set<SCCset *> *getChildren(SCCset *subset);
    std::vector<SCCset *> &getDepthOrderedSubsets () {
      return depthOrderedSubsets;
    }

    raw_ostream &print (raw_ostream &stream, std::string prefix);
    std::string subsetStr (SCCset *subset);
    raw_ostream &printSCCIndices (raw_ostream &stream, std::string prefix);
    raw_ostream &printNodeInGraph (raw_ostream &stream, std::string prefix, SCCset *subset);
    raw_ostream &printGraph (raw_ostream &stream, std::string prefix);

  private:

    void resetSubsetGraph ();
    void collectSubsetGraph ();
    bool hasCycle ();
    void orderSubsets ();

    bool mergeCycles ();
    bool traverseAndMerge (std::vector<SCCset *> &path);

    /*
     * Subset mapping
     */
    std::set<SCCset *> *subsets;
    std::unordered_map<SCC *, SCCset *> SCCToSet;
    std::vector<SCC *> SCCDebugOrder;
    std::unordered_map<SCC *, int> SCCDebugIndex;

    /*
     * Mappings at the end point in history
     */
    std::set<SCCset *> roots;
    std::unordered_map<SCCset *, std::set<SCCset *>> parentSubsets;
    std::unordered_map<SCCset *, std::set<SCCset *>> childrenSubsets;

    std::unordered_map<SCCset *, int> subsetDepths;
    std::vector<SCCset *> depthOrderedSubsets;

    /*
     * Static reference information
     */
    SCCDAG *sccdag;
    SCCDAGAttrs *dagAttrs;
    LoopsSummary *LIS;
};
