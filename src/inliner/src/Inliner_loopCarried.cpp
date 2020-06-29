/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Inliner.hpp"

bool Inliner::inlineCallsInvolvedInLoopCarriedDataDependences (Noelle &noelle) {
  auto anyInlined = false;

  /*
   * Order these functions to prevent duplicating loops yet to be checked
   */
  std::vector<Function *> orderedFns;
  for (auto fnLoops : loopsToCheck) {
    orderedFns.push_back(fnLoops.first);
  }
  sortInDepthOrderFns(orderedFns);

  std::set<Function *> fnsToAvoid;
  for (auto F : orderedFns) {
    
    /*
     * If we avoid this function until next pass, we do the same with its parents
     */
    if (fnsToAvoid.find(F) != fnsToAvoid.end()) {
      for (auto parentF : parentFns[F]) {
        fnsToAvoid.insert(parentF);
      }
      continue;
    }

    /*
     * Fetch all loops of the current function.
     */
    auto allLoops = noelle.getLoops(F);

    /*
     * Sort the loops by size.
     * This makes the loops sorted by scope as well (from outer to inner loops).
     */
    noelle.sortByStaticNumberOfInstructions(*allLoops);

    /*
     * Fetch the set of loops that are enabled.
     */
    auto &toCheck = loopsToCheck[F];

    /*
     * Inline calls that are involved in loop-carried data dependences for the enabled loops.
     */
    auto inlined = false;
    std::set<LoopStructure *> removeSummaries;
    for (auto LDI : *allLoops){

      /*
       * Check if the current loop has been enabled.
       */
      LoopStructure *summary = nullptr;
      for (auto enabledLoop : toCheck){
        if (enabledLoop->getHeader() == LDI->getLoopStructure()->getHeader()){
          summary = enabledLoop;
          break ;
        }
      }
      if (summary == nullptr){
        continue ;
      }

      /*
       * Inline the call.
       */
      auto inlinedCall = this->inlineCallsInMassiveSCCs(F, LDI);
      inlined |= inlinedCall;
      if (inlined) break;
    }

    /*
     * Free the memory.
     */
    for (auto tempLDI : *allLoops){
      delete tempLDI;
    }
    delete allLoops ;

    /*
     * Keep track of the inlining.
     */
    anyInlined |= inlined;

    /*
     * Avoid parents of affected functions.
     * This is because we are not finished with the affected functions.
     */
    if (inlined) {
      for (auto parentF : parentFns[F]) {
        fnsToAvoid.insert(parentF);
      }
    }
  }
    
  return anyInlined;
}

/*
 * GOAL: Go through loops in function
 * If there is only one non-clonable/reducable SCC,
 * try inlining the function call in that SCC with the
 * most memory edges to other internal/external values
 */
bool Inliner::inlineCallsInMassiveSCCs (Function *F, LoopDependenceInfo *LDI) {

  /*
   * Fetch the SCCDAG
   */
  auto SCCDAG = LDI->sccdagAttrs.getSCCDAG();

  /*
   * Fetch the loop structure.
   */
  auto loopStructure = LDI->getLoopStructure();

  std::set<SCC *> sccsToCheck;
  SCCDAG->iterateOverSCCs([LDI, &sccsToCheck](SCC *scc) -> bool{
    auto sccInfo = LDI->sccdagAttrs.getSCCAttrs(scc);
    if (sccInfo->mustExecuteSequentially()){
      sccsToCheck.insert(scc);
    }
    return false;
  });

  /*
   * Check every sequential SCC.
   */
  int64_t maxMemEdges = 0;
  CallInst *inlineCall = nullptr;
  for (auto scc : sccsToCheck) {

    /*
     * Check every instruction within the sequential SCC.
     */
    for (auto valNode : scc->getNodes()) {

      /*
       * Fetch the next call instruction of the current sequential SCC.
       */
      auto val = valNode->getT();
      if (!isa<CallInst>(val)) {
        continue ;
      }
      auto call = cast<CallInst>(val);

      /*
       * Fetch the callee.
       */
      auto callF = call->getCalledFunction();
      if (!callF){

        /*
         * The callee is unknown.
         * So we cannot inline this call.
         */
        continue ;
      }
      if (callF->empty()) {

        /*
         * The callee's body is unknown (it is a library function).
         * So we cannot inline this call.
         */
        continue;
      }

      /*
       * Do not consider inlining a recursive function call
       */
      if (callF == F) {
        continue;
      }

      /*
       * Do not consider inlining calls to functions of lower depth
       */
      if (fnOrders[callF] < fnOrders[F]) {
        continue;
      }

      /*
       * If the call instruction belongs to a sub-loop, then its inlining is likely to be useless.
       */
      if (loopStructure->isIncludedInItsSubLoops(call)){
        continue ;
      }

      /*
       * Count how many memory edges this call is involved in.
       */
      auto memEdgeCount = 0;
      for (auto edge : valNode->getAllConnectedEdges()) {
        if (edge->isMemoryDependence()) {
          memEdgeCount++;
        }
      }

      /*
       * Consider only the call instruction with the maximum number of memory edges.
       */
      if (memEdgeCount > maxMemEdges) {
        maxMemEdges = memEdgeCount;
        inlineCall = call;
      }
    }
  }

  /*
   * Check if there is a function to inline.
   */
  if (inlineCall == nullptr){

    /*
     * No call needs to be inlined.
     */
    return false;
  }

  /*
   * Inline the call instruction.
   */
  auto inlined = inlineFunctionCall(F, inlineCall->getCalledFunction(), inlineCall);

  return inlined;
}
