/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Inliner.hpp"
#include "DOALL.hpp"

namespace llvm::noelle {

bool Inliner::inlineCallsInvolvedInLoopCarriedDataDependences (Noelle &noelle, noelle::CallGraph *pcg) {
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
      StayConnectedNestedLoopForestNode *summaryNode = nullptr;
      LoopStructure *summary = nullptr;
      for (auto enabledLoop : toCheck){
        if (enabledLoop->getHeader() == LDI->getLoopStructure()->getHeader()){
          summary = enabledLoop;
          summaryNode = LDI->getLoopHierarchyStructures();
          break ;
        }
      }
      if (summary == nullptr){
        continue ;
      }

      /*
       * Check if the current loop is a DOALL.
       * If it is, then we disable all sub-loops to be considered because DOALL always takes priority and we don't parallelize nested loops at the moment.
       */
      DOALL doall{
        noelle
      };
      if (  true
            && (summaryNode->getNumberOfSubLoops() >= 1)
            && doall.canBeAppliedToLoop(LDI, nullptr)
        ){

        /*
         * The loop is a doall.
         *
         * Disable all sub-loops
         */
        auto disableSubLoop = [&toCheck] (const LoopStructure &child) -> bool{

          /*
           * Check if the sub-loop is enabled.
           */
          if (std::find(toCheck.begin(), toCheck.end(), &child) == toCheck.end()){
            return false;
          }

          /*
           * The sub-loop is enabled.
           *
           * Disable it.
           */
          std::remove(toCheck.begin(), toCheck.end(), &child);

          return false;
        };
        LDI->iterateOverSubLoopsRecursively(disableSubLoop);

        continue ;
      }

      /*
       * Inline the call.
       */
      auto inlinedCall = this->inlineCallsInvolvedInLoopCarriedDataDependencesWithinLoop(F, LDI, pcg, noelle);
      inlined |= inlinedCall;
      if (inlined) {
        break;
      }
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
bool Inliner::inlineCallsInvolvedInLoopCarriedDataDependencesWithinLoop (
    Function *F, 
    LoopDependenceInfo *LDI, 
    noelle::CallGraph *pcg,
    Noelle &noelle
    ) {
  assert(pcg != nullptr);
  assert(LDI != nullptr);

  /*
   * Fetch the profile
   */
  auto hot = noelle.getProfiles();

  /*
   * Fetch the SCC manager.
   */
  auto sccManager = LDI->getSCCManager();

  /*
   * Fetch the SCCDAG
   */
  auto SCCDAG = sccManager->getSCCDAG();

  /*
   *inlineFunctionCall Fetch the loop structure.
   */
  auto loopStructure = LDI->getLoopStructure();
  auto loopStructureNode = LDI->getLoopHierarchyStructures();

  /*
   * Check every sequential SCC.
   */
  int64_t maxMemEdges = 0;
  uint32_t numberOfFunctionCallsToInline = 0;
  CallInst *inlineCall = nullptr;
  auto nonDOALLSCCs = DOALL::getSCCsThatBlockDOALLToBeApplicable(LDI, noelle);
  for (auto scc : nonDOALLSCCs) {

    /*
     * Check every instruction within the sequential SCC.
     *
     * Do not inline a call that depends on itself because it is unlikely to make a difference.
     * Most of the time such situation shows up as an SCC with a single node.
     */
    if (scc->numberOfInstructions() == 1){
      continue ;
    }

    /*
     * The SCC includes more than one instruction.
     * Check its calls.
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
       * Check if the callee is an intrinsic.
       */
      if (callF->isIntrinsic()){
        continue ;
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
      if (loopStructureNode->isIncludedInItsSubLoops(call)){
        continue ;
      }

      /*
       * Do not consider inlining calls that are in a cycle within the program call graph.
       */
      if (pcg->doesItBelongToASCC(callF)){
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
       * Also, consider only calls to functions that are smaller than the current loop size.
       */
      numberOfFunctionCallsToInline++;
      if (  true
            && (memEdgeCount > maxMemEdges) 
            && (hot->getStaticInstructions(callF) < hot->getStaticInstructions(loopStructure))
        ){
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
   * Check if there are too many loop-carried data dependences related to function calls.
   */
  if (numberOfFunctionCallsToInline >= this->maxNumberOfFunctionCallsToInlinePerLoop){
    errs() << "Inliner:   The loop " << *loopStructure->getHeader()->getFirstNonPHI() << " has too many function calls involved in loop-carried data dependences (there are " << numberOfFunctionCallsToInline << ")\n";
    return false;
  }

  /*
   * Inline the call instruction.
   */
  auto inlined = inlineFunctionCall(hot, F, inlineCall->getCalledFunction(), inlineCall);

  return inlined;
}

}
