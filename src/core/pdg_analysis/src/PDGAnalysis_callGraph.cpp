/*
 * Copyright 2016 - 2021  Angelo Matni, Yian Su, Simone Campanoni
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
#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/TalkDown.hpp"
#include "noelle/core/PDGPrinter.hpp"
#include "noelle/core/PDGAnalysis.hpp"
#include "IntegrationWithSVF.hpp"

namespace llvm::noelle {

noelle::CallGraph *PDGAnalysis::getProgramCallGraph(void) {

  /*
   * Compute the call graph.
   */
  if (this->noelleCG == nullptr) {
    this->noelleCG = NoelleSVFIntegration::getProgramCallGraph(*M);
  }

  /*
   * Check if external call graph analyses have been registered.
   */
  if (this->cgAnalyses.size() > 0){

    /*
     * Improve the call graph using external call graph analyses.
     */
    for (auto node : this->noelleCG->getFunctionNodes()){
      for (auto outgoingEdge : node->getOutgoingEdges()){
        
        /*
         * We can only improve may edges.
         */
        if (outgoingEdge->isAMustCall()){
          continue ;
        }

        /*
         * Fetch the callee.
         */
        auto calleeNode = outgoingEdge->getCallee();
        auto callee = calleeNode->getFunction();

        /*
         * The current edge is a function->function edge.
         * We need to iterate over the sub-edges to see the instructions that are responsible to this edge.
         */
        std::set<CallGraphInstructionFunctionEdge *> toDelete{};
        CallGraphInstructionFunctionEdge *mustSubEdge = nullptr;
        auto subedges = outgoingEdge->getSubEdges();
        for (auto subedge : subedges){
          assert(!subedge->isAMustCall());

          /*
           * Fetch the caller of this specific sub-edge.
           */
          auto callerNode = subedge->getCaller();
          auto caller = cast<CallBase>(callerNode->getInstruction());

          /*
           * Query the external analyses.
           */
          for (auto cga : this->cgAnalyses){
            auto queryResult = cga->canThisFunctionBeACallee(caller, *callee);
            if (queryResult == CS_CANNOT_EXIST){
              toDelete.insert(subedge);
              break ;
            } else if (queryResult == CS_MUST_EXIST){
              mustSubEdge = subedge;
              break ;
            }
          }
          if (toDelete.size() > 0){
            assert(mustSubEdge == nullptr);
          }
        }
      }
    }
  }

  return this->noelleCG;
}

void PDGAnalysis::identifyFunctionsThatInvokeUnhandledLibrary(Module &M) {

  /*
   * Collect internal and unhandled external functions.
   */
  for (auto &F : M) {
    if (F.empty()) {
      if (this->externalFuncsHaveNoSideEffectOrHandledBySVF.count(
              F.getName())) {
        continue;
      }
      this->unhandledExternalFuncs.insert(&F);
    } else {
      this->internalFuncs.insert(&F);
    }
  }

  /*
   * Identify function reachability.
   */
  for (auto &internal : this->internalFuncs) {
    for (auto &external : this->unhandledExternalFuncs) {
      if (NoelleSVFIntegration::isReachableBetweenFunctions(internal,
                                                            external)) {
        this->reachableUnhandledExternalFuncs[internal].insert(external);
      }
    }
  }

  return;
}

bool PDGAnalysis::cannotReachUnhandledExternalFunction(CallBase *call) {
  if (NoelleSVFIntegration::hasIndCSCallees(call)) {
    auto callees = NoelleSVFIntegration::getIndCSCallees(call);
    for (auto &callee : callees) {
      if (this->isUnhandledExternalFunction(callee)
          || isInternalFunctionThatReachUnhandledExternalFunction(callee))
        return false;
    }

  } else {
    auto callee = call->getCalledFunction();
    if (!callee || this->isUnhandledExternalFunction(callee)
        || isInternalFunctionThatReachUnhandledExternalFunction(callee))
      return false;
  }

  return true;
}

bool PDGAnalysis::isUnhandledExternalFunction(const Function *F) {
  return F->empty()
         && !this->externalFuncsHaveNoSideEffectOrHandledBySVF.count(
             F->getName());
}

bool PDGAnalysis::isInternalFunctionThatReachUnhandledExternalFunction(
    const Function *F) {
  return !F->empty() && !this->reachableUnhandledExternalFuncs[F].empty();
}

} // namespace llvm::noelle
