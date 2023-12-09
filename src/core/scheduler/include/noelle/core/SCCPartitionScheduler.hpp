/*
 * Copyright 2019 - 2020  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef NOELLE_SRC_CORE_SCHEDULER_SCCPARTITIONSCHEDULER_H_
#define NOELLE_SRC_CORE_SCHEDULER_SCCPARTITIONSCHEDULER_H_
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/Dominators.hpp"
#include "noelle/core/LoopDependenceInfo.hpp"
#include "noelle/core/DataFlowResult.hpp"
#include "noelle/core/SCCDAGPartition.hpp"

namespace arcana::noelle {

class SCCPartitionScheduler {
public:
  SCCPartitionScheduler(SCCDAG *loopSCCDAG,
                        std::unordered_set<SCCSet *> sccPartitions,
                        DataFlowResult *reachabilityDFR);

  bool squeezePartitions(void);

private:
  SCCDAG *loopSCCDAG;
  std::unordered_set<SCCSet *> sccPartitions;

  /*
   * The reachability DFR OUT set for an instruction I is
   * all instructions J that can be reached from I.
   * The reverse reachability is the OUT set of the inverse DFR.
   * For an instruction I, the OUT set would be all J that can reach I
   */
  DataFlowResult *reachabilityDFR;
  std::unordered_map<Instruction *, std::unordered_set<Instruction *>>
      reverseReachabilityMap;

  std::unordered_map<SCC *, SCCSet *> sccToPartitionMap;
  std::unordered_map<BasicBlock *, std::unordered_set<SCCSet *>>
      basicBlockToPartitionsMap;

  SCCSet *getPartition(Instruction *I);

  Instruction *hoistInstructionTowards(Instruction *instructionToMove,
                                       Instruction *target);
  Instruction *sinkInstructionTowards(Instruction *instructionToMove,
                                      Instruction *target);

  std::unordered_set<Instruction *> collectDependedOnInstructionsWithinBlock(
      Instruction *I);
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_SCHEDULER_SCCPARTITIONSCHEDULER_H_
