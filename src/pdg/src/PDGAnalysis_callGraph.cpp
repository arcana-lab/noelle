/*
 * Copyright 2016 - 2020  Angelo Matni, Yian Su, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "SystemHeaders.hpp"

#include "Util/SVFModule.h"
#include "WPA/Andersen.h"
#include "TalkDown.hpp"
#include "PDGPrinter.hpp"
#include "PDGAnalysis.hpp"

using namespace llvm;

noelle::CallGraph * PDGAnalysis::getProgramCallGraph (void){
  auto cg = new noelle::CallGraph(*M, this->callGraph);

  return cg;
}

void PDGAnalysis::identifyFunctionsThatInvokeUnhandledLibrary(Module &M) {

  /*
   * Collect internal and unhandled external functions.
   */
  for (auto &F : M) {
    if (F.empty()) {
      if (this->externalFuncsHaveNoSideEffectOrHandledBySVF.count(F.getName())) {
        continue;
      }
      this->unhandledExternalFuncs.insert(&F);
    }
    else {
      this->internalFuncs.insert(&F);
    }
  }

  /*
   * Identify function reachability.
   */
  for (auto &internal : this->internalFuncs) {
    for (auto &external : this->unhandledExternalFuncs) {
      if (this->callGraph->isReachableBetweenFunctions(internal, external)) {
        this->reachableUnhandledExternalFuncs[internal].insert(external);
      }
    }
  }

  return;
}

bool PDGAnalysis::cannotReachUnhandledExternalFunction(CallInst *call) {
  if (this->callGraph->hasIndCSCallees(call)) {
    const set<const Function *> callees = this->callGraph->getIndCSCallees(call);
    for (auto &callee : callees) {
      if (this->isUnhandledExternalFunction(callee) || isInternalFunctionThatReachUnhandledExternalFunction(callee)) return false;
    }

  } else {
    auto callee = call->getCalledFunction();
    if (!callee || this->isUnhandledExternalFunction(callee) || isInternalFunctionThatReachUnhandledExternalFunction(callee)) return false;
  }

  return true;
}

bool PDGAnalysis::isUnhandledExternalFunction(const Function *F) {
  return F->empty() && !this->externalFuncsHaveNoSideEffectOrHandledBySVF.count(F->getName());
}

bool PDGAnalysis::isInternalFunctionThatReachUnhandledExternalFunction(const Function *F) {
  return !F->empty() && !this->reachableUnhandledExternalFuncs[F].empty();
}
