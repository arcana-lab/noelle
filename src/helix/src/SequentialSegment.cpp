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
   * Fetch the header.
   */
  auto loopSummary = LDI->getLoopStructure();
  auto loopHeader = loopSummary->getHeader();

  /*
   * Fetch the loop function.
   */
  auto loopFunction = loopSummary->getFunction();

  /*
   * Set the ID
   */
  this->ID = ID;

  /*
   * Track the SCCset for later optimizations
   */
  this->sccs = sccs;

  /*
   * Identify all dependent instructions that require synchronization
   */
  if (this->verbosity >= Verbosity::Maximal) {
    errs() << "HELIX:   Sequential segment " << ID << "\n" ;
    errs() << "HELIX:     SCCs included in the current sequential segment\n";
  }
  std::set<Instruction *> ssInstructions;
  for (auto scc : *sccs){

    // NOTE: SCC sandwiched between two with cycles may be single instruction nodes without sycles.
    // assert(scc->hasCycle());

    if (this->verbosity >= Verbosity::Maximal) {

      /*
       * Fetch the SCC metadata.
       */
      auto sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);

      /*
       * Print
       */
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
    errs() << "HELIX:     Instructions that belong to the SS\n";
    for (auto ssInst : ssInstructions){
      errs() << "HELIX:       " << *ssInst << "\n";
    }
  }

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
  auto dfr = dfa.applyBackward(loopFunction, computeGEN, computeKILL, computeIN, computeOUT);

  /*
   * For instructions in the SS, derive the set of instructions containing it in their reachable list
   * Alternatively, a second data flow analysis could be performed to achieve this
   */
  std::unordered_map<Instruction *, std::unordered_set<Instruction *>> beforeInstructionMap{};
  for (auto ssInst : ssInstructions) {
    std::unordered_set<Instruction *> empty;
    beforeInstructionMap.insert(std::make_pair(ssInst, empty));
  }
  for (auto ssInst : ssInstructions) {
    auto &afterInstructions = dfr->OUT(ssInst);
    for (auto afterV : afterInstructions) {
      auto afterI = cast<Instruction>(afterV);
      if (ssInst == afterI) continue;
      if (ssInstructions.find(afterI) == ssInstructions.end()) continue;

      beforeInstructionMap.at(afterI).insert(ssInst);
    }
  }

  /*
   * Use beforeInstructionMap to determine all entries to the SS
   * All entries cannot be reached by any other instruction in the SS
   */
  for (auto pair : beforeInstructionMap) {
    auto entryInst = pair.first;
    auto &beforeInstructions = pair.second;
    if (beforeInstructions.size() != 0) continue;

    /*
     * NOTE: Do not include PHIs in a sequential segment. Let the PHI redirect data properly before
     * entry into the sequential segment. This knowledge would be lost after the entry because
     * of the insertion of control flow checking whether to wait before entering the segment
     */
    auto firstI = (!isa<PHINode>(entryInst) && !isa<DbgInfoIntrinsic>(entryInst) && !entryInst->isLifetimeStartOrEnd())
      ? entryInst : entryInst->getParent()->getFirstNonPHIOrDbgOrLifetime();
    this->entries.insert(firstI);
  }

  /*
   * Use afterInstructions result from data flow analysis to determine all exits to the SS
   * All exits cannot reach any other instruction in the SS
   */
  for (auto ssInst : ssInstructions) {
    auto &afterInstructions = dfr->OUT(ssInst);

    bool hasAfterInstructions = false;
    for (auto afterV : afterInstructions) {
      auto afterI = cast<Instruction>(afterV);
      if (ssInst == afterI) continue;

      if (ssInstructions.find(afterI) != ssInstructions.end()) {
        hasAfterInstructions = true;
        break;
      }
    }
    if (hasAfterInstructions) continue;

    /*
     * NOTE: Include all PHIs of a basic block before the exit of a sequential segment.
     * No instruction, such as the 'signal' call instruction at the end of a sequential segment,
     * can be placed before all PHIs of a basic block
     */
    auto lastI = (!isa<PHINode>(ssInst) && !isa<DbgInfoIntrinsic>(ssInst) && !ssInst->isLifetimeStartOrEnd())
      ? ssInst : ssInst->getParent()->getFirstNonPHIOrDbgOrLifetime();
    this->exits.insert(lastI);
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
