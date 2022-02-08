/*
 * Copyright 2016 - 2022  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"
#include "SequentialSegment.hpp"

namespace llvm::noelle {

SequentialSegment::SequentialSegment (
  Noelle &noelle,
  LoopDependenceInfo *LDI, 
  DataFlowResult *reachabilityDFR,
  SCCSet *sccs,
  int32_t ID,
  Verbosity verbosity
  ) :
  verbosity{verbosity}
  {

  /*
   * Set the loop function, header, ID, and SCC set of the SS
   */
  auto loopStructure = LDI->getLoopStructure();
  auto loopFunction = loopStructure->getFunction();
  auto loopHeader = loopStructure->getHeader();
  this->ID = ID;
  this->sccs = sccs;

  /*
   * Compute dominator information
   */
  auto ds = noelle.getDominators(loopFunction);

  /*
   * Identify all dependent instructions that require synchronization
   * NOTE: Exclude PHINode instructions (TODO: determine other instructions
   * that do not have any influence on the defining of entry/exit frontiers)
   */
  auto ssInstructions = this->getInstructions();
  std::unordered_set<Instruction *> excludedInstructions;
  for (auto I : ssInstructions) {
    if (!isa<PHINode>(I)) continue;
    excludedInstructions.insert(I);
  }
  for (auto excludedI : excludedInstructions) {
    ssInstructions.erase(excludedI);
  }
  if (this->verbosity >= Verbosity::Maximal) {
    printSCCInfo(LDI, ssInstructions);
  }

  /*
   * Identify all possible entry and exit points of the sequential segment.
   */
  this->determineEntryAndExitFrontier(LDI, ds, reachabilityDFR, ssInstructions);

  /* 
   * NOTE: Function-exiting blocks, even if in nested loops, are the exception to the rule that all
   * waits/signals must not be contained in a sub-loop as they only execute once
   */
  for (auto B : loopStructure->getBasicBlocks()) {
    if (succ_size(B) != 0) continue;
    auto instructionThatReturnsFromFunction = B->getTerminator();
    this->exits.insert(instructionThatReturnsFromFunction);
  }

  assert(this->entries.size() > 0
    && "The data flow analysis did not identify any per-iteration entry to the sequential segment!\n");
  assert(this->exits.size() > 0
    && "The data flow analysis did not identify any per-iteration exit to the sequential segment!\n");

  return ;
}

void SequentialSegment::forEachEntry (
  std::function <void (Instruction *justAfterEntry)> whatToDo){

  /*
   * Iterate over the entries.
   */
  for (auto entry : this->entries){
    whatToDo(entry);
  }

  return ;
}

void SequentialSegment::forEachExit (
  std::function <void (Instruction *justBeforeExit)> whatToDo){

  /*
   * Iterate over the exits.
   */
  for (auto exit : this->exits){
    whatToDo(exit);
  }

  return ;
}

int32_t SequentialSegment::getID (void){
  return this->ID;
}

void SequentialSegment::determineEntryAndExitFrontier (
  LoopDependenceInfo *LDI,
  DominatorSummary *DS,
  DataFlowResult *dfr,
  std::unordered_set<Instruction *> &ssInstructions
) {

  /*
   * Fetch the loop
   */
  auto rootLoop = LDI->getLoopStructure();
  auto beforeInstructionMap = this->computeBeforeInstructionMap(LDI, dfr);

  /*
   * Instructions from which no other instruction in the SS can reach them are before the entry frontier
   */
  auto checkIfBeforeEntryFrontier = [&](Instruction *inst) -> bool {
    auto &beforeInstructions = beforeInstructionMap.at(inst);
    for (auto beforeI : beforeInstructions) {
      if (beforeI == inst) continue;
      if (ssInstructions.find(beforeI) != ssInstructions.end()) return false;
    }
    return true;
  };

  /*
   * Instructions from which no other instruction in the SS can be reached are after the exit frontier
   */
  auto checkIfAfterExitFrontier = [&](Instruction *inst) -> bool {
    auto &afterInstructions = dfr->OUT(inst);
    for (auto afterV : afterInstructions) {
      auto afterI = cast<Instruction>(afterV);
      if (inst == afterI) continue;
      if (ssInstructions.find(afterI) != ssInstructions.end()) return false;
    }
    return true;
  };

  /*
   * Checking if an entry dominates this instruction is used to prevent
   * marking unnecessary entries. This is an optimization.
   */
  auto isDominatedByOtherEntry = [&](Instruction *inst) -> bool {
    for (auto entry : this->entries) {
      if (DS->DT.dominates(entry, inst)) return true;
    }
    return false;
  };

  /*
   * Checking if an exit can be reached from this instruction is used to prevent
   * exit synchronization from being triggered more than once. This is for correctness.
   */
  auto isReachableFromOtherExit = [&](Instruction *inst) -> bool {
    auto &beforeInstructions = beforeInstructionMap.at(inst);
    for (auto beforeI : beforeInstructions) {
      if (this->exits.find(beforeI) != this->exits.end()) return true;
    }
    return false;
  };

  /*
   * Traverse all SS instructions and their predecessors in search for entries to form a frontier
   * We consider an instruction an entry if
   * 1) the instruction is NOT in a subloop
   * 2) no SS instruction can reach it within the given loop iteration
   * 3) no other entry dominates this entry
   */
  std::queue<Instruction *> predecessorTraversal;
  std::unordered_set<Instruction *> predecessorVisited;
  for (auto ssInst : ssInstructions) {
    predecessorTraversal.push(ssInst);
  }
  while (!predecessorTraversal.empty()) {
    auto potentialEntryI = predecessorTraversal.front();
    predecessorTraversal.pop();

    /*
     * Ensure we do not re-visit a node; that would be a waste of time
     */
    if (predecessorVisited.find(potentialEntryI) != predecessorVisited.end()) continue;
    predecessorVisited.insert(potentialEntryI);

    /*
     * Check if this is a valid entry. If so, do not proceed along its predecessors
     */
    auto nestedMostLoopOfI = LDI->getNestedMostLoopStructure(potentialEntryI);
    if (true
      && nestedMostLoopOfI == rootLoop
      && checkIfBeforeEntryFrontier(potentialEntryI)
      && !isDominatedByOtherEntry(potentialEntryI)
    ) {
      auto nonInterferingEntryPoint = getFrontierInstructionThatDoesNotSplitPHIs(potentialEntryI);
      this->entries.insert(nonInterferingEntryPoint);
      continue;
    }

    /*
     * Proceed along all predecessors to ensure a complete frontier is found
     * If there is a previous instruction, add that to the queue
     * Else, add all predecessor basic block's terminators
     */
    auto prevInst = potentialEntryI->getPrevNonDebugInstruction();
    if (prevInst) {
      predecessorTraversal.push(prevInst);
      continue;
    }
    auto B = potentialEntryI->getParent();
    for (auto predecessor : predecessors(B)) {
      auto terminator = predecessor->getTerminator();
      predecessorTraversal.push(terminator);
    }
  }

  /*
   * Traverse all SS instructions and their successors in search for exits to form a frontier
   * We consider an instruction an exit if
   * 1) the instruction is NOT in a subloop
   * 2) no SS instruction can be reached from it within the given loop iteration
   * 3) no other exit can be reached from this exit within the given loop iteration
   */
  std::queue<Instruction *> successorTraversal;
  std::unordered_set<Instruction *> successorVisited;
  for (auto ssInst : ssInstructions) {
    successorTraversal.push(ssInst);
  }
  while (!successorTraversal.empty()) {
    auto potentialExitI = successorTraversal.front();
    successorTraversal.pop();

    /*
     * Ensure we do not re-visit a node; that would be a waste of time
     */
    if (successorVisited.find(potentialExitI) != successorVisited.end()) continue;
    successorVisited.insert(potentialExitI);

    /*
     * Check if this is a valid exit. If so, do not proceed along its successors
     */
    auto nestedMostLoopOfI = LDI->getNestedMostLoopStructure(potentialExitI);
    if (true
      && nestedMostLoopOfI == rootLoop
      && checkIfAfterExitFrontier(potentialExitI)
      && !isReachableFromOtherExit(potentialExitI)
    ) {
      auto nonInterferingExitPoint = getFrontierInstructionThatDoesNotSplitPHIs(potentialExitI);
      this->exits.insert(nonInterferingExitPoint);
      continue;
    }

    /*
     * Proceed along all successors to ensure a complete frontier is found
     * If there is a following instruction, add that to the queue
     * Else, add all successor block's first instructions
     */
    auto nextInst = potentialExitI->getNextNonDebugInstruction();
    if (nextInst) {
      successorTraversal.push(nextInst);
      continue;
    }
    auto B = potentialExitI->getParent();
    for (auto successor : successors(B)) {
      auto firstInst = &*successor->begin();
      successorTraversal.push(firstInst);
    }
  }

  /*
   * Extend the entry and exit frontier to cut through the entire CFG, not missing any branches
   * going around SS instructions. This is done by choosing from a set of instructions which
   * no SS instruction can reach and from which no SS instruction can be reached.
   *
   * The subset chosen to extend the entry frontier will not be dominated by any
   * other element in the set
   *
   * The subset chosen to extend the exit frontier cannot reach or be reached by any
   * other element in the set
   *
   * All basic blocks in the loop must be considered to ensure the frontier is fully encompassing
   *
   * First, find all instructions in the set of un-reachables. Only one instruction per basic
   * block is needed to represent this set sufficiently
   */
  std::unordered_set<Instruction *> instructionsUnreachableToAndFromSS;
  for (auto B : rootLoop->getBasicBlocks()) {
    auto I = &*B->begin();

    // The condition ensuring I is not a member of SS is in case the SS only has 1 instruction,
    // which then causes checkIfBeforeEntryFrontier and checkIfAfterExitFrontier to return true
    if (true
      && (ssInstructions.find(I) == ssInstructions.end())
      && checkIfBeforeEntryFrontier(I)
      && checkIfAfterExitFrontier(I)
    ) {
      instructionsUnreachableToAndFromSS.insert(I);
    }
  }

  /*
   * Go through all unreachable instructions in search for necessary entries/exits
   */
  for (auto I : instructionsUnreachableToAndFromSS) {
    auto nonInterferingPoint = getFrontierInstructionThatDoesNotSplitPHIs(I);
    if (!isDominatedByOtherEntry(nonInterferingPoint)) {
      this->entries.insert(nonInterferingPoint);
    }
    if (!isReachableFromOtherExit(nonInterferingPoint)) {
      this->exits.insert(nonInterferingPoint);
    }
  }

  return ;
}

/*
 * Do not separate PHIs with sequential segment boundaries. Let the PHIs redirect data properly before
 * entry (where a wait is added) or before exit (where a signal is added).
 */
Instruction * SequentialSegment::getFrontierInstructionThatDoesNotSplitPHIs (Instruction *originalBarrierInst) {
  return (false
    || isa<PHINode>(originalBarrierInst)
    || isa<DbgInfoIntrinsic>(originalBarrierInst)
    || originalBarrierInst->isLifetimeStartOrEnd()
  )
    ? originalBarrierInst->getParent()->getFirstNonPHIOrDbgOrLifetime()
    : originalBarrierInst;
}

/*
 * For each instruction I in the loop, derive the set of instructions J that could have been executed before I.
 * This is accomplished by considering each instruction in the OUT reachable set of I as instructions that could execute before J.
 */
std::unordered_map<Instruction *, std::unordered_set<Instruction *>> SequentialSegment::computeBeforeInstructionMap (
  LoopDependenceInfo *LDI,
  DataFlowResult *dfr
) {

  /*
   * Initialize the output data structure.
   */
  auto loopStructure = LDI->getLoopStructure();
  std::unordered_map<Instruction *, std::unordered_set<Instruction *>> beforeInstructionMap{};
  for (auto B : loopStructure->getBasicBlocks()) {
    for (auto &I : *B) {
      std::unordered_set<Instruction *> empty;
      beforeInstructionMap.insert(std::make_pair(&I, empty));
    }
  }

  /*
   * Compute the output.
   */
  for (auto B : loopStructure->getBasicBlocks()) {
    for (auto &I : *B) {

      /*
       * Fetch the instructions that are reachable starting from I.
       */
      auto &afterInstructions = dfr->OUT(&I);

      /*
       * Use the reachable instruction-information to compute the output.
       */
      for (auto afterV : afterInstructions) {
        auto afterI = cast<Instruction>(afterV);
        if (&I == afterI) continue;
        if (!loopStructure->isIncluded(afterI)) continue;

        beforeInstructionMap.at(afterI).insert(&I);
      }
    }
  }

  return beforeInstructionMap;
}

DataFlowResult * HELIX::computeReachabilityFromInstructions (LoopDependenceInfo *LDI) {

  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto loopFunction = loopStructure->getFunction();

  /*
   * Run the data flow analysis needed to identify the locations where signal instructions will be placed.
   */
  auto dfa = this->noelle.getDataFlowEngine();
  auto computeGEN = [](Instruction *i, DataFlowResult *df) {
    auto& gen = df->GEN(i);
    gen.insert(i);
    return ;
  };
  auto computeOUT = [LDI, loopHeader](std::set<Value *>& OUT, Instruction *succ, DataFlowResult *df) {

    /*
     * Check if the successor is the header.
     * In this case, we do not propagate the reachable instructions.
     * We do this because we are interested in understanding the reachability of instructions within a single iteration.
     */
    auto succBB = succ->getParent();
    if (succ == &*loopHeader->begin()) {
      return ;
    }

    /*
     * Propagate the data flow values.
     */
    auto& inS = df->IN(succ);
    OUT.insert(inS.begin(), inS.end());
    return ;
  } ;
  auto computeIN = [](std::set<Value *>& IN, Instruction *inst, DataFlowResult *df) {
    auto& genI = df->GEN(inst);
    auto& outI = df->OUT(inst);
    IN.insert(outI.begin(), outI.end());
    IN.insert(genI.begin(), genI.end());
    return ;
  };

  return dfa.applyBackward(loopFunction, computeGEN, computeIN, computeOUT);
}

iterator_range<std::unordered_set<SCC *>::iterator> SequentialSegment::getSCCs(void) {
  return make_range(sccs->sccs.begin(), sccs->sccs.end());
}

std::unordered_set<Instruction *> SequentialSegment::getInstructions (void) {
  std::unordered_set<Instruction *> ssInstructions;
  for (auto scc : sccs->sccs){

    /*
     * Add all instructions of the current SCC to the set.
     */
    for (auto nodePair : scc->internalNodePairs()){
      auto inst = cast<Instruction>(nodePair.first);

      /*
       * Fetch the LLVM value associated to the node
       * NOTE: Values internal to an SCC are instructions
       */
      ssInstructions.insert(inst);
    }
  }

  return ssInstructions;
}

void SequentialSegment::printSCCInfo (LoopDependenceInfo *LDI, std::unordered_set<Instruction *> &ssInstructions) {

  errs() << "HELIX:   Sequential segment " << ID << "\n" ;
  errs() << "HELIX:     SCCs included in the current sequential segment\n";

  auto sccManager = LDI->getSCCManager();
  for (auto scc : sccs->sccs){

    auto sccInfo = sccManager->getSCCAttrs(scc);

    errs() << "HELIX:       Type = " << sccInfo->getType() << "\n";
    errs() << "HELIX:       Loop-carried data dependences\n";
    auto lcIterFunc = [scc](DGEdge<Value> *dep) -> bool {
      auto fromInst = dep->getOutgoingT();
      auto toInst = dep->getIncomingT();
      assert(scc->isInternal(fromInst) || scc->isInternal(toInst));
      errs() << "HELIX:        \"" << *fromInst << "\" -> \"" << *toInst  << "\"\n";
      return false;
    };
    sccManager->iterateOverLoopCarriedDataDependences(scc, lcIterFunc);
  }

  errs() << "HELIX:     Instructions that belong to the SS\n";
  for (auto ssInst : ssInstructions){
    errs() << "HELIX:       " << *ssInst << "\n";
  }

  return;
}

}
