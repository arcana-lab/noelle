/*
 * Copyright 2016 - 2023  Angelo Matni, Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_LDG_ANALYSIS_LOOPITERATIONSPACEANALYSIS_H_
#define NOELLE_SRC_CORE_LDG_ANALYSIS_LOOPITERATIONSPACEANALYSIS_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/SCCDAG.hpp"
#include "arcana/noelle/core/SCC.hpp"
#include "arcana/noelle/core/InductionVariables.hpp"
#include "arcana/noelle/core/ScalarEvolutionDelinearization.hpp"
#include "arcana/noelle/core/LoopGoverningInductionVariable.hpp"
#include "arcana/noelle/core/IVStepperUtility.hpp"

namespace arcana::noelle {

class LoopIterationSpaceAnalysis {
public:
  LoopIterationSpaceAnalysis(LoopTree *loops,
                             InductionVariableManager &ivManager,
                             ScalarEvolution &SE);

  LoopIterationSpaceAnalysis() = delete;

  bool areInstructionsAccessingDisjointMemoryLocationsBetweenIterations(
      Instruction *from,
      Instruction *to) const;

  ~LoopIterationSpaceAnalysis();

private:
  class MemoryAccessSpace {
  public:
    MemoryAccessSpace(Instruction *memoryAccessor);

    /*
     * Instruction accessing memory, such as IntToPtrInst or GetElementPtrInst
     * NOTE: We look for instructions, as Value alone isn't helpful for domain
     * space analysis
     */
    Instruction *memoryAccessor;
    const SCEV *memoryAccessorSCEV;
    const SCEV *memoryAccessorBasePointerSCEV;
    const SCEV *memoryMinusSCEV;

    /*
     * Were we able to determine the boundaries of this memory space?
     */
    bool isAnalyzed;

    /*
     * For linear spaces, track each dimension's access SCEVs
     */
    SmallVector<const SCEV *, 4> subscripts;
    SmallVector<const SCEV *, 4> sizes;
    const SCEV *elementSize;

    /*
     * Track the instruction and the IV corresponding to each subscript
     * This instruction may either be
     * 1) directly represented by the IV's SCEV: {0,+,1}
     * 2) derived from that IV's SCEV, for example: ({0,+,1} + 3) * 2
     */
    SmallVector<std::pair<Instruction *, InductionVariable *>, 4> subscriptIVs;
  };

  /*
   * Long-lived references
   */
  LoopTree *loops;
  InductionVariableManager &ivManager;

  /*
   * Associate SCEVs with all IV instructions matching that evolution
   */
  std::unordered_map<const SCEV *, std::unordered_set<Instruction *>>
      ivInstructionsBySCEV;
  std::unordered_map<const SCEV *, std::unordered_set<Instruction *>>
      derivedInstructionsFromIVsBySCEV;
  std::unordered_map<Instruction *, InductionVariable *> ivsByInstruction;

  /*
   * For memory accessing instructions with pointer operands that are known
   * linear SCEVs, track the access space for the instruction
   */
  std::unordered_set<std::unique_ptr<MemoryAccessSpace>> accessSpaces;
  std::unordered_map<Instruction *, MemoryAccessSpace *>
      accessSpaceByInstruction;
  std::unordered_map<MemoryAccessSpace *, std::set<MemoryAccessSpace *>>
      spacesThatCannotOverlap;

  /*
   * Cache memory access spaces with certain properties
   */
  std::unordered_set<MemoryAccessSpace *>
      nonOverlappingAccessesBetweenIterations;

  /*
   * Methods
   */
  void indexIVInstructionSCEVs(ScalarEvolution &SE);

  void computeMemoryAccessSpace(ScalarEvolution &SE);

  void identifyIVForMemoryAccessSubscripts(ScalarEvolution &SE);

  void identifyNonOverlappingAccessesBetweenIterationsAcrossOneLoopInvocation(
      ScalarEvolution &SE);

  bool isMemoryAccessSpaceEquivalentForTopLoopIVSubscript(
      MemoryAccessSpace *space1,
      MemoryAccessSpace *space2) const;

  bool isOneToOneFunctionOnIV(LoopStructure *LS,
                              InductionVariable *IV,
                              Instruction *derivedInstruction);

  bool isInnerDimensionSubscriptsBounded(ScalarEvolution &SE,
                                         MemoryAccessSpace *space);

  bool analyzeToCheckIfMemoryAccessSpaceNotOverlappingOrExactlyTheSame(
      MemoryAccessSpace *accessSpaceI,
      MemoryAccessSpace *accessSpaceJ) const;

  bool areMemoryAccessSpaceNotOverlappingOrExactlyTheSame(
      MemoryAccessSpace *accessSpaceI,
      MemoryAccessSpace *accessSpaceJ) const;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_LDG_ANALYSIS_LOOPITERATIONSPACEANALYSIS_H_
