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
#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/TalkDown.hpp"
#include "arcana/noelle/core/PDGPrinter.hpp"
#include "noelle/core/PDGGenerator.hpp"
#include "IntegrationWithSVF.hpp"

namespace arcana::noelle {

noelle::CallGraph *PDGGenerator::getProgramCallGraph(void) {

  /*
   * Compute the call graph.
   */
  if (this->noelleCG == nullptr) {
    if (this->disableSVFCallGraph) {
      auto hasF = [](CallBase *call) -> bool {
        if (call->getCalledFunction() == nullptr) {
          return true;
        }
        return false;
      };
      auto getCallees = [this](CallBase *call) -> std::set<const Function *> {
        /*
         * Check if @call is a direct call.
         */
        auto calleeF = call->getCalledFunction();
        if (calleeF != nullptr) {
          return { calleeF };
        }

        /*
         * @call is an indirect call.
         */
        auto callees = PDGGenerator::getFunctionsThatMightEscape(*M);
        auto targetSignature = call->getFunctionType();
        auto compatibleCallees =
            PDGGenerator::getFunctionsWithSignature(callees, targetSignature);

        return compatibleCallees;
      };
      this->noelleCG = new noelle::CallGraph(*M, hasF, getCallees);

    } else {
      this->noelleCG = NoelleSVFIntegration::getProgramCallGraph(*M);
    }
  }

  /*
   * Check if external call graph analyses have been registered.
   */
  if (this->cgAnalyses.size() > 0) {

    /*
     * Improve the call graph using external call graph analyses.
     */
    for (auto node : this->noelleCG->getFunctionNodes()) {
      for (auto outgoingEdge : this->noelleCG->getOutgoingEdges(node)) {

        /*
         * Fetch the callee.
         */
        auto calleeNode = outgoingEdge->getCallee();
        auto callee = calleeNode->getFunction();

        /*
         * The current edge is a function->function edge.
         * We need to iterate over the sub-edges to see the instructions that
         * are responsible to this edge.
         */
        std::set<CallGraphInstructionFunctionEdge *> toDelete{};
        CallGraphInstructionFunctionEdge *mustSubEdge = nullptr;
        auto subedges = outgoingEdge->getSubEdges();
        for (auto subedge : subedges) {

          /*
           * We can only improve may edges.
           */
          if (subedge->isAMustCall()) {
            continue;
          }

          /*
           * Fetch the caller of this specific sub-edge.
           */
          auto callerNode = subedge->getCaller();
          auto caller = cast<CallBase>(callerNode->getInstruction());

          /*
           * Query the external analyses.
           */
          for (auto cga : this->cgAnalyses) {
            auto queryResult = cga->canThisFunctionBeACallee(caller, *callee);
            if (queryResult == CS_CANNOT_EXIST) {
              toDelete.insert(subedge);
              break;
            } else if (queryResult == CS_MUST_EXIST) {
              mustSubEdge = subedge;
              break;
            }
          }
          if (toDelete.size() > 0) {

            /*
             * External analyses have identified the current sub-edge to be
             * removable.
             */
            assert(mustSubEdge == nullptr);
            this->noelleCG->removeSubEdge(outgoingEdge, subedge);
          }
          if (mustSubEdge != nullptr) {

            /*
             * External analyses have identified the current sub-edge to be a
             * must edge.
             */
            subedge->setMust();
            outgoingEdge->setMust();
          }
        }
      }
    }
  }

  return this->noelleCG;
}

void PDGGenerator::identifyFunctionsThatInvokeUnhandledLibrary(Module &M) {

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

bool PDGGenerator::cannotReachUnhandledExternalFunction(CallBase *call) {
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

bool PDGGenerator::isUnhandledExternalFunction(const Function *F) {
  return F->empty()
         && !this->externalFuncsHaveNoSideEffectOrHandledBySVF.count(
             F->getName());
}

bool PDGGenerator::isInternalFunctionThatReachUnhandledExternalFunction(
    const Function *F) {
  return !F->empty() && !this->reachableUnhandledExternalFuncs[F].empty();
}

std::set<const Function *> PDGGenerator::getFunctionsWithSignature(
    std::set<const Function *> functions,
    FunctionType *signature) {
  std::set<const Function *> compatibleCallees;
  for (auto f : functions) {
    if (f->getFunctionType() == signature) {
      compatibleCallees.insert(f);
    }
  }

  return compatibleCallees;
}

std::set<const Function *> PDGGenerator::getFunctionsThatMightEscape(
    Module &currentProgram) {
  std::set<const Function *> callees;

  /*
   * Collect all functions that escape and that are compatible with the
   * signature of the call instruction.
   */
  for (auto &F : currentProgram) {

    /*
     * Check if @F is used for something that isn't a direct call.
     * In this case, we cannot exclude (without additional analysis) that @F
     * could be invoked indirectly.
     */
    for (auto user : F.users()) {
      if (auto c = dyn_cast<CallBase>(user)) {
        if (c->getCalledFunction() == &F) {
          continue;
        }
      }

      /*
       * @F could be invoked indirectly as its address is used by a non-call
       * instruction.
       */
      callees.insert(&F);
    }
  }

  return callees;
}

} // namespace arcana::noelle
