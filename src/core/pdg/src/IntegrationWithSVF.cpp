/*
 * Copyright 2020 - 2021  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/SystemHeaders.hpp"
#include "IntegrationWithSVF.hpp"

/*
 * SVF headers
 */
#define ENABLE_SVF
#ifdef ENABLE_SVF
#include "WPA/WPAPass.h"
#include "Util/SVFModule.h"
#include "Util/PTACallGraph.h"
#include "WPA/Andersen.h"
#include "MemoryModel/PointerAnalysis.h"
#include "MSSA/MemSSA.h"
#endif

namespace llvm::noelle {

#ifdef ENABLE_SVF
static WPAPass *wpa = nullptr;
static MemSSA *mssa = nullptr;
static PointerAnalysis *pta = nullptr;
static PTACallGraph *svfCallGraph = nullptr;
#endif

// Next there is code to register your pass to "opt"
char NoelleSVFIntegration::ID = 0;
static RegisterPass<NoelleSVFIntegration> X("noellesvf", "Integration with SVF");

// Next there is code to register your pass to "clang"
static NoelleSVFIntegration * _PassMaker = nullptr;
static RegisterStandardPasses _RegPass1(PassManagerBuilder::EP_OptimizerLast,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new NoelleSVFIntegration()); }}); // ** for -Ox
static RegisterStandardPasses _RegPass2(PassManagerBuilder::EP_EnabledOnOptLevel0,
    [](const PassManagerBuilder&, legacy::PassManagerBase& PM) {
        if(!_PassMaker){ PM.add(_PassMaker = new NoelleSVFIntegration()); }});// ** for -O0

NoelleSVFIntegration::NoelleSVFIntegration ()
  : ModulePass{ID}
{
  return ;
}

bool NoelleSVFIntegration::doInitialization (Module &M) {
  return false;
}

void NoelleSVFIntegration::getAnalysisUsage(AnalysisUsage &AU) const {
  #ifdef ENABLE_SVF
  AU.addRequired<WPAPass>();
  #endif
  return ;
}
      
bool NoelleSVFIntegration::runOnModule (Module &M) {
  #ifdef ENABLE_SVF
  // get SVF's WPAPass analysis for all applicable pointer analysis
  wpa = &getAnalysis<WPAPass>();

  // run a single AndersenWaveDiff pointer analysis manually for querying ModRef info
  SVFModule svfModule{M};
  pta = new AndersenWaveDiff();
  pta->analyze(svfModule);
  svfCallGraph = pta->getPTACallGraph();
  mssa = new MemSSA((BVDataPTAImpl *)pta, false);
  #endif

  return false;
}
    
noelle::CallGraph * NoelleSVFIntegration::getProgramCallGraph (Module &M) {

  /*
   * Compute the call graph using NOELLE
   */
  auto cg = new noelle::CallGraph(M, NoelleSVFIntegration::hasIndCSCallees, NoelleSVFIntegration::getIndCSCallees);

  return cg;
}
    
bool NoelleSVFIntegration::hasIndCSCallees (CallBase *call) {
  #ifdef ENABLE_SVF
  if (auto callInst = dyn_cast<CallInst>(call)){
    return svfCallGraph->hasIndCSCallees(callInst);
  }
  return true;
  #else
  if (call->getCalledFunction() == nullptr){
    return true;
  }
  return false;
  #endif
}
    
const std::set<const Function *> NoelleSVFIntegration::getIndCSCallees (CallBase *call){
  #ifdef ENABLE_SVF
  if (auto callInst = dyn_cast<CallInst>(call)){
    return svfCallGraph->getIndCSCallees(callInst);
  }
  //TODO
  return {};
  #else
  //TODO
  return {};
  #endif
}
    
bool NoelleSVFIntegration::isReachableBetweenFunctions (const Function *from, const Function *to){
  #ifdef ENABLE_SVF
  return svfCallGraph->isReachableBetweenFunctions(from, to);
  #else
  return true;
  #endif
}
    
ModRefInfo NoelleSVFIntegration::getModRefInfo (CallBase *i){
  #ifdef ENABLE_SVF
  if (auto callInst = dyn_cast<CallInst>(i)){
    return mssa->getMRGenerator()->getModRefInfo(callInst);
  }
  return ModRefInfo::ModRef;
  #else
  return ModRefInfo::ModRef;
  #endif
}
    
ModRefInfo NoelleSVFIntegration::getModRefInfo (CallBase *i, const MemoryLocation &loc){
  #ifdef ENABLE_SVF
  if (auto callInst = dyn_cast<CallInst>(i)){
    return mssa->getMRGenerator()->getModRefInfo(callInst, loc);
  }
  return ModRefInfo::ModRef;
  #else
  return ModRefInfo::ModRef;
  #endif
}
    
ModRefInfo NoelleSVFIntegration::getModRefInfo (CallBase *i, CallBase *j){
  #ifdef ENABLE_SVF
  auto callInstI = dyn_cast<CallInst>(i);
  auto callInstJ = dyn_cast<CallInst>(j);
  if (  true
        && (callInstI != nullptr)
        && (callInstJ != nullptr)
     ){
    return mssa->getMRGenerator()->getModRefInfo(callInstI, callInstJ);
  }
  return ModRefInfo::ModRef;
  #else
  return ModRefInfo::ModRef;
  #endif
}

AliasResult NoelleSVFIntegration::alias (const MemoryLocation &loc1, const MemoryLocation &loc2){
  #ifdef ENABLE_SVF
  return wpa->alias(loc1, loc2);
  #else
  return AliasResult::MayAlias;
  #endif
}

AliasResult NoelleSVFIntegration::alias (const Value *v1, const Value *v2){
  #ifdef ENABLE_SVF
  return wpa->alias(v1, v2);
  #else
  return AliasResult::MayAlias;
  #endif
}

}
