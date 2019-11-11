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
    assert(scc->hasCycle());
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
  auto dfa = DataFlowAnalysis{};
  auto computeGEN = [](Instruction *i, DataFlowResult *df) {
    auto& gen = df->GEN(i);
    gen.insert(i);
    return ;
  };
  auto computeKILL = [](Instruction *, DataFlowResult *) {
    return ;
  };
  auto computeOUT = [LDI](std::set<Value *>& OUT, Instruction *succ, DataFlowResult *df) {

    /*
     * Check if the successor is the header.
     * In this case, we do not propagate the reachable instructions.
     * We do this because we are interested in understanding the reachability of instructions within a single iteration.
     */
    auto succBB = succ->getParent();
    if (succ == &*LDI->header->begin()) {
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
  auto dfr = dfa.applyBackward(LDI->function, computeGEN, computeKILL, computeIN, computeOUT);

  /*
   * Identify the locations where signal and wait instructions should be placed.
   */
  std::list<Instruction *> workingList;
  std::unordered_map<Instruction *, bool> visited;
  for (auto I : ssInstructions) {
    visited[I] = true;
    workingList.push_back(I);
  }
  while (!workingList.empty()){

    /*
     * Fetch the current instruction to consider.
     */
    auto I = workingList.front();
    workingList.pop_front();
    assert(visited[I] == true);

    /*
     * Check OUT[I]
     */
    auto &afterInstructions = dfr->OUT(I);
    std::set<Instruction *> inSS;
    for (auto afterV : afterInstructions) {
      auto afterI = cast<Instruction>(afterV);
      if (I == afterI) continue;

      if (ssInstructions.find(afterI) != ssInstructions.end()) {
        inSS.insert(afterI);
      }
    }

    /*
     * Check if I is an exit of the current sequential segment.
     */
    bool noneInSS = inSS.size() == 0;
    if (noneInSS) {
      this->exits.insert(I);
      continue ;
    }

    /*
     *
     * Add the successors of I to the working list.
     */
    auto bb = I->getParent();
    if (bb->getTerminator() != I){

      /*
       * I is inside a basic block.
       *
       * Fetch the next instruction within the same basic block.
       */
      BasicBlock::iterator iter(I);
      iter++;
      auto succI = &*iter;
      if (visited.find(succI) == visited.end()){
        workingList.push_back(succI);
        visited[succI] = true;
      }

    } else {

      /*
       * I is the terminator of a basic block.
       * We need to add the first instructions of the basic block successors if they belong to the loop.
       */
      for (auto succBB : successors(bb)){

        /*
         * Check if succBB belongs to the loop being parallelized.
         */
        if (std::find(LDI->loopBBs.begin(), LDI->loopBBs.end(), succBB) == LDI->loopBBs.end()){

          /*
           * succBB doesn't belong to the loop being parallelized.
           */
          continue ;
        }

        /*
         * succBB belongs to the loop being parallelized.
         */
        auto succI = succBB->getFirstNonPHIOrDbgOrLifetime();
        if (visited.find(succI) == visited.end()){
          workingList.push_back(succI);
          visited[succI] = true;
        }
      }
    }

    /*
     * Check if I is an entry of the current sequential segment.
     */
    bool allInSS = (inSS.size() + 1) == ssInstructions.size();
    if (  true
          && allInSS
          && (ssInstructions.find(I) != ssInstructions.end())
          ) {
      this->entries.insert(I);
    }

    /*
     * I is not an entry and it is not an exit.
     */
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
