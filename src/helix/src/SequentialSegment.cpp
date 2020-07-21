/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "HELIX.hpp"
#include "SequentialSegment.hpp"
#include "DataFlow.hpp"

using namespace llvm ;

SequentialSegment::SequentialSegment (
  LoopDependenceInfo *LDI, 
  SCCset *sccs,
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
   * Identify all dependent instructions that require synchronization
   */
  std::set<Instruction *> ssInstructions;
  for (auto scc : *sccs){

    // NOTE: SCC sandwiched between two with cycles may be single instruction nodes without sycles.
    // assert(scc->hasCycle());

    /*
     * Add all instructions of the current SCC to the set.
     */
    for (auto nodePair : scc->internalNodePairs()){

      /*
       * Fetch the LLVM value associated to the node
       * NOTE: Values internal to an SCC are instructions
       */
      ssInstructions.insert(cast<Instruction>(nodePair.first));
    }
  }
  if (this->verbosity >= Verbosity::Maximal) {
    printSCCInfo(LDI, ssInstructions);
  }

  auto dfr = this->computeReachabilityFromInstructions(LDI);
  determineEntriesAndExits(LDI, dfr, ssInstructions);

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

void SequentialSegment::determineEntriesAndExits (
  LoopDependenceInfo *LDI,
  DataFlowResult *dfr,
  std::set<Instruction *> &ssInstructions
) {

  auto loopStructure = LDI->getLoopStructure();

  /*
   * For each instruction I in the loop, derive the set of instructions J "before" it,
   * where each instruction of J is in the reachable set from I
   * Alternatively, a second data flow analysis could be performed to achieve this
   */
  std::unordered_map<Instruction *, std::unordered_set<Instruction *>> beforeInstructionMap{};
  std::unordered_set<Instruction *> returningInstructions;
  for (auto B : loopStructure->getBasicBlocks()) {

    if (succ_size(B) == 0) {
      auto terminator = B->getTerminator();
      returningInstructions.insert(terminator);
    }

    for (auto &I : *B) {
      std::unordered_set<Instruction *> empty;
      beforeInstructionMap.insert(std::make_pair(&I, empty));
    }
  }

  for (auto B : loopStructure->getBasicBlocks()) {
    for (auto &I : *B) {
      auto &afterInstructions = dfr->OUT(&I);
      for (auto afterV : afterInstructions) {
        auto afterI = cast<Instruction>(afterV);
        if (&I == afterI) continue;
        if (!loopStructure->isIncluded(afterI)) continue;

        beforeInstructionMap.at(afterI).insert(&I);
      }
    }
  }

  /* 
   * NOTE: Loop-exiting blocks, even if in nested loops, are the exception to the rule that all
   * waits/signals must not be contained in a sub-loop as they only execute once
   */
  for (auto returningInst : returningInstructions) {
    this->exits.insert(returningInst);
  }

  /*
   * NOTE: Do not separate PHIs with sequential segment boundaries. Let the PHIs redirect data properly before
   * entry (where a wait is added) or before exit (where a signal is added).
  */
  auto getInstructionThatDoesNotSplitPHIs = [&](Instruction *originalBarrierInst) -> Instruction * {
    return (!isa<PHINode>(originalBarrierInst)
      && !isa<DbgInfoIntrinsic>(originalBarrierInst)
      && !originalBarrierInst->isLifetimeStartOrEnd())
        ? originalBarrierInst
        : originalBarrierInst->getParent()->getFirstNonPHIOrDbgOrLifetime();
  };

  /*
   * Entries are instructions from which no other instruction in the SS can reach them
   */
  auto checkIfEntry = [&](Instruction *inst) -> bool {
    auto &beforeInstructions = beforeInstructionMap.at(inst);
    for (auto beforeI : beforeInstructions) {
      if (beforeI == inst) continue;
      if (ssInstructions.find(beforeI) != ssInstructions.end()) return false;
    }
    return true;
  };

  /*
   * Exits are instructions from which no other instruction in the SS can be reached by them
   */
  auto checkIfExit = [&](Instruction *inst) -> bool {
    auto &afterInstructions = dfr->OUT(inst);
    for (auto afterV : afterInstructions) {
      auto afterI = cast<Instruction>(afterV);
      if (inst == afterI) continue;
      if (ssInstructions.find(afterI) != ssInstructions.end()) return false;
    }
    return true;
  };

  auto addEntry = [&](Instruction *entry) -> void {
    auto firstI = getInstructionThatDoesNotSplitPHIs(entry);
    this->entries.insert(firstI);
  };

  auto addExit = [&](Instruction *exit) -> void {
    auto lastI = getInstructionThatDoesNotSplitPHIs(exit);
    this->exits.insert(lastI);
  };

  /*
   * Attempt to see if entry and/or exit SS instructions can be found strictly using reachability.
   * This is the case if entries/exits are not contained within sub-loops
   */
  for (auto ssInst : ssInstructions) {
    if (!checkIfEntry(ssInst)) continue;
    addEntry(ssInst);
  }
  for (auto ssInst : ssInstructions) {
    if (!checkIfExit(ssInst)) continue;
    addExit(ssInst);
  }
  if (this->entries.size() > 0 && this->exits.size() > 0) return;

  /*
   * If all potential entries and/or exits are in sub-loops,
   * determine the following blocks contained only by the parallelized loop:
   *   "entry" blocks: the set of precedessor blocks to all SS instructions
   *   "exit" blocks: the set of successor blocks to all SS instructions
   */
  if (this->entries.size() == 0) {
    std::queue<BasicBlock *> blocksPrecedingSSInsts{};
    std::unordered_set<BasicBlock *> blocksVisited{};
    for (auto ssInst : ssInstructions) {
      auto block = ssInst->getParent();
      blocksPrecedingSSInsts.push(block);
      blocksVisited.insert(block);
    }

    while (!blocksPrecedingSSInsts.empty()) {
      auto block = blocksPrecedingSSInsts.front();
      blocksPrecedingSSInsts.pop();

      auto firstInst = &*block->begin();
      if (!loopStructure->isIncluded(firstInst)) continue;

      auto nestedMostLoop = LDI->getNestedMostLoopStructure(firstInst);
      auto isEntry = checkIfEntry(firstInst);
      if (nestedMostLoop == loopStructure && isEntry) {
        addEntry(firstInst);
        continue;
      }

      for (auto predBlock : predecessors(block)) {
        if (blocksVisited.find(predBlock) != blocksVisited.end()) continue;
        blocksPrecedingSSInsts.push(predBlock);
        blocksVisited.insert(predBlock);
      }
    }
  }

  if (this->exits.size() == 0) {
    std::queue<BasicBlock *> blocksFollowingSSInsts{};
    std::unordered_set<BasicBlock *> blocksVisited{};
    for (auto ssInst : ssInstructions) {
      auto block = ssInst->getParent();
      blocksFollowingSSInsts.push(block);
      blocksVisited.insert(block);
    }

    while (!blocksFollowingSSInsts.empty()) {
      auto block = blocksFollowingSSInsts.front();
      blocksFollowingSSInsts.pop();

      auto terminator = block->getTerminator();
      if (!loopStructure->isIncluded(terminator)) continue;

      auto nestedMostLoop = LDI->getNestedMostLoopStructure(terminator);
      auto isExit = checkIfExit(terminator);
      if (nestedMostLoop == loopStructure && isExit) {
        addExit(terminator);
        continue;
      }

      for (auto succBlock : successors(block)) {
        if (blocksVisited.find(succBlock) != blocksVisited.end()) continue;
        blocksFollowingSSInsts.push(succBlock);
        blocksVisited.insert(succBlock);
      }
    }
  }

  return;
}

DataFlowResult *SequentialSegment::computeReachabilityFromInstructions (LoopDependenceInfo *LDI) {

  auto loopStructure = LDI->getLoopStructure();
  auto loopHeader = loopStructure->getHeader();
  auto loopFunction = loopStructure->getFunction();

  /*
   * Run the data flow analysis needed to identify the locations where signal instructions will be placed.
   */
  auto dfa = DataFlowEngine{};
  auto computeGEN = [](Instruction *i, DataFlowResult *df) {
    auto& gen = df->GEN(i);
    gen.insert(i);
    return ;
  };
  auto computeKILL = [](Instruction *, DataFlowResult *) {
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

  return dfa.applyBackward(loopFunction, computeGEN, computeKILL, computeIN, computeOUT);
}

void SequentialSegment::printSCCInfo (LoopDependenceInfo *LDI, std::set<Instruction *> &ssInstructions) {

  errs() << "HELIX:   Sequential segment " << ID << "\n" ;
  errs() << "HELIX:     SCCs included in the current sequential segment\n";

  for (auto scc : *sccs){

    auto sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);

    errs() << "HELIX:       Type = " << sccInfo->getType() << "\n";
    errs() << "HELIX:       Loop-carried data dependences\n";
    auto lcIterFunc = [scc](DGEdge<Value> *dep) -> bool {
      auto fromInst = dep->getOutgoingT();
      auto toInst = dep->getIncomingT();
      assert(scc->isInternal(fromInst) || scc->isInternal(toInst));
      errs() << "HELIX:        \"" << *fromInst << "\" -> \"" << *toInst  << "\"\n";
      return false;
    };
    LDI->sccdagAttrs.iterateOverLoopCarriedDataDependences(scc, lcIterFunc);
  }

  errs() << "HELIX:     Instructions that belong to the SS\n";
  for (auto ssInst : ssInstructions){
    errs() << "HELIX:       " << *ssInst << "\n";
  }

  return;
}
