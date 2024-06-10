/*
 * Copyright 2020 - 2021  Simone Campanoni
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
#include "noelle/core/ProgramAliasAnalysisEngine.hpp"
#include "arcana/noelle/core/PDGGenerator.hpp"
#include "IntegrationWithSVF.hpp"

/*
 * SVF headers
 */
#ifdef NOELLE_ENABLE_SVF
#  include "WPA/WPAPass.h"
#  include "Util/SVFModule.h"
#  include "Util/PTACallGraph.h"
#  include "WPA/Andersen.h"
#  include "MemoryModel/PointerAnalysis.h"
#  include "MSSA/MemSSA.h"
#endif

namespace arcana::noelle {

void PDGGenerator::initializeSVF(Module &M) {
  return;
}

#ifdef NOELLE_ENABLE_SVF
static WPAPass *wpa = nullptr;
static MemSSA *mssa = nullptr;
static PointerAnalysis *pta = nullptr;
static PTACallGraph *svfCallGraph = nullptr;
#endif

// Next there is code to register your pass to "opt"
char NoelleSVFIntegration::ID = 0;
static RegisterPass<NoelleSVFIntegration> X("noelle-svf",
                                            "Integration with SVF");

NoelleSVFIntegration::NoelleSVFIntegration() : ModulePass{ ID } {
  return;
}

bool NoelleSVFIntegration::doInitialization(Module &M) {
  return false;
}

void NoelleSVFIntegration::getAnalysisUsage(AnalysisUsage &AU) const {
#ifdef NOELLE_ENABLE_SVF
  AU.addRequired<WPAPass>();
#endif
  return;
}

bool NoelleSVFIntegration::runOnModule(Module &M) {
#ifdef NOELLE_ENABLE_SVF
  // get SVF's WPAPass analysis for all applicable pointer analysis
  wpa = &getAnalysis<WPAPass>();

  // run a single AndersenWaveDiff pointer analysis manually for querying ModRef
  // info
  SVFModule svfModule{ M };
  pta = new AndersenWaveDiff();
  pta->analyze(svfModule);
  svfCallGraph = pta->getPTACallGraph();
  mssa = new MemSSA((BVDataPTAImpl *)pta, false);
#endif

  return false;
}

noelle::CallGraph *NoelleSVFIntegration::getProgramCallGraph(Module &M) {

  /*
   * Compute the call graph using NOELLE
   */
  auto cg = new noelle::CallGraph(M,
                                  NoelleSVFIntegration::hasIndCSCallees,
                                  NoelleSVFIntegration::getIndCSCallees);

  return cg;
}

bool NoelleSVFIntegration::hasIndCSCallees(CallBase *call) {
#ifdef NOELLE_ENABLE_SVF
  if (auto callInst = dyn_cast<CallInst>(call)) {
    return svfCallGraph->hasIndCSCallees(callInst);
  }
  return true;
#else
  if (call->getCalledFunction() == nullptr) {
    return true;
  }
  return false;
#endif
}

const std::set<const Function *> NoelleSVFIntegration::getIndCSCallees(
    CallBase *call) {

  /*
   * Check if @call is a direct call.
   */
  auto calleeF = call->getCalledFunction();
  if (calleeF != nullptr) {
    return { calleeF };
  }

  /*
   * @call is an indirect call
   *
   * Check if we SVF has been enabled and can handle @call.
   */
#ifdef NOELLE_ENABLE_SVF
  if (auto callInst = dyn_cast<CallInst>(call)) {
    return svfCallGraph->getIndCSCallees(callInst);
  }
#endif

  /*
   * SVF has not been enabled or it cannot handle @call.
   *
   * Collect all functions that escape and that are compatible with the
   * signature of the call instruction.
   */
  auto currentProgram = call->getModule();
  auto callees = PDGGenerator::getFunctionsThatMightEscape(*currentProgram);
  auto targetSignature = call->getFunctionType();
  auto compatibleCallees =
      PDGGenerator::getFunctionsWithSignature(callees, targetSignature);

  return compatibleCallees;
}

bool NoelleSVFIntegration::isReachableBetweenFunctions(const Function *from,
                                                       const Function *to) {
#ifdef NOELLE_ENABLE_SVF
  return svfCallGraph->isReachableBetweenFunctions(from, to);
#else
  return true;
#endif
}

ModRefInfo NoelleSVFIntegration::getModRefInfo(CallBase *i) {
#ifdef NOELLE_ENABLE_SVF
  if (auto callInst = dyn_cast<CallInst>(i)) {
    return mssa->getMRGenerator()->getModRefInfo(callInst);
  }
  return ModRefInfo::ModRef;
#else
  return ModRefInfo::ModRef;
#endif
}

ModRefInfo NoelleSVFIntegration::getModRefInfo(CallBase *i,
                                               const MemoryLocation &loc) {
#ifdef NOELLE_ENABLE_SVF
  if (auto callInst = dyn_cast<CallInst>(i)) {
    return mssa->getMRGenerator()->getModRefInfo(callInst, loc);
  }
  return ModRefInfo::ModRef;
#else
  return ModRefInfo::ModRef;
#endif
}

ModRefInfo NoelleSVFIntegration::getModRefInfo(CallBase *i, CallBase *j) {
#ifdef NOELLE_ENABLE_SVF
  auto callInstI = dyn_cast<CallInst>(i);
  auto callInstJ = dyn_cast<CallInst>(j);
  if (true && (callInstI != nullptr) && (callInstJ != nullptr)) {
    return mssa->getMRGenerator()->getModRefInfo(callInstI, callInstJ);
  }
  return ModRefInfo::ModRef;
#else
  return ModRefInfo::ModRef;
#endif
}

AliasResult NoelleSVFIntegration::alias(const MemoryLocation &loc1,
                                        const MemoryLocation &loc2) {
#ifdef NOELLE_ENABLE_SVF
  return wpa->alias(loc1, loc2);
#else
  return AliasResult::MayAlias;
#endif
}

AliasResult NoelleSVFIntegration::alias(const Value *v1, const Value *v2) {
#ifdef NOELLE_ENABLE_SVF
  return wpa->alias(v1, v2);
#else
  return AliasResult::MayAlias;
#endif
}

std::set<AliasAnalysisEngine *> PDGGenerator::getProgramAliasAnalysisEngines(
    void) {
  std::set<AliasAnalysisEngine *> s;

#ifdef NOELLE_ENABLE_SVF
  auto svf = new ProgramAliasAnalysisEngine("SVF", wpa);
  s.insert(svf);
#endif

  return s;
}

} // namespace arcana::noelle
