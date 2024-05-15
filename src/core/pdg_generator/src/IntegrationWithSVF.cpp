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
#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/ProgramAliasAnalysisEngine.hpp"
#include "noelle/core/PDGGenerator.hpp"
#include "IntegrationWithSVF.hpp"

/*
 * SVF headers
 */
#ifdef NOELLE_ENABLE_SVF
#  include "Util/Options.h"
#  include "SVF-LLVM/LLVMModule.h"
#  include "SVF-LLVM/SVFIRBuilder.h"
#  include "SVFIR/SVFIR.h"
#  include "SVFIR/SVFValue.h"
#  include "WPA/WPAPass.h"
#  include "WPA/Andersen.h"
#  include "Graphs/PTACallGraph.h"
#  include "Graphs/ICFG.h"
#  include "MSSA/MemSSA.h"
#endif

namespace arcana::noelle {

#ifdef NOELLE_ENABLE_SVF
static SVF::WPAPass *wpa = nullptr;
static SVF::PTACallGraph *svfCallGraph = nullptr;
static SVF::ICFG *icfg = nullptr;
static SVF::MemSSA *mssa = nullptr;
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
  return;
}

bool NoelleSVFIntegration::runOnModule(Module &M) {
#ifdef NOELLE_ENABLE_SVF
  /*
   * Select SVF options
   */
  // Pointer Analysis
  SVF::Options::PASelected.parseAndSetValue("nander");
  SVF::Options::PASelected.parseAndSetValue("sander");
  SVF::Options::PASelected.parseAndSetValue("sfrander");
  SVF::Options::PASelected.parseAndSetValue("ander");
  // Alias analyis rule: return NoAlias if any pta says no alias
  SVF::Options::AliasRule.parseAndSetValue("veto");
  // Disable SVF stats
  SVF::Options::PStat.setValue(false);

  /*
   * Build SVFIR
   */
  SVF::SVFModule *svfM = SVF::LLVMModuleSet::buildSVFModule(M);
  SVF::SVFIRBuilder svfIRBuilder(svfM);
  SVF::SVFIR *svfIR = svfIRBuilder.build();

  /*
   * Run SVF's whole program analysis
   */
  wpa = new SVF::WPAPass();
  wpa->runOnModule(svfIR);

  /*
   * Run a single AndersenWaveDiff pointer analysis for querying ModRefInfo
   */
  SVF::Andersen *ander = SVF::AndersenWaveDiff::createAndersenWaveDiff(svfIR);
  ander->analyze();
  svfCallGraph = ander->getPTACallGraph();
  icfg = svfIR->getICFG();
  mssa = new SVF::MemSSA((SVF::BVDataPTAImpl *)ander, false);
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
    SVF::SVFValue *val =
        SVF::LLVMModuleSet::getLLVMModuleSet()->getSVFValue(callInst);
    SVF::CallSite callsite = SVF::SVFUtil::getSVFCallSite(val);
    SVF::CallICFGNode *icfgNode =
        icfg->getCallICFGNode(callsite.getInstruction());
    return svfCallGraph->hasIndCSCallees(icfgNode);
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
    SVF::SVFValue *val =
        SVF::LLVMModuleSet::getLLVMModuleSet()->getSVFValue(callInst);
    SVF::CallSite callsite = SVF::SVFUtil::getSVFCallSite(val);
    SVF::CallICFGNode *icfgNode =
        icfg->getCallICFGNode(callsite.getInstruction());
    SVF::Set<const SVF::SVFFunction *> svfFunctions =
        svfCallGraph->getIndCSCallees(icfgNode);
    std::set<const Function *> callees;
    for (auto svfFunction : svfFunctions) {
      const Function *function = static_cast<const Function *>(
          SVF::LLVMModuleSet::getLLVMModuleSet()->getLLVMValue(svfFunction));
      callees.insert(function);
    }
    return callees;
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
  SVF::SVFFunction *svfFn1 =
      SVF::LLVMModuleSet::getLLVMModuleSet()->getSVFFunction(from);
  SVF::SVFFunction *svfFn2 =
      SVF::LLVMModuleSet::getLLVMModuleSet()->getSVFFunction(to);
  return svfCallGraph->isReachableBetweenFunctions(svfFn1, svfFn2);
#else
  return true;
#endif
}

ModRefInfo NoelleSVFIntegration::getModRefInfo(CallBase *i) {
#ifdef NOELLE_ENABLE_SVF
  if (auto callInst = dyn_cast<CallInst>(i)) {
    SVF::SVFValue *svfVal =
        SVF::LLVMModuleSet::getLLVMModuleSet()->getSVFValue(callInst);
    SVF::CallSite callsite = SVF::SVFUtil::getSVFCallSite(svfVal);
    SVF::CallICFGNode *icfgNode =
        icfg->getCallICFGNode(callsite.getInstruction());
    switch (mssa->getMRGenerator()->getModRefInfo(icfgNode)) {
      case SVF::ModRefInfo::NoModRef:
        return llvm::ModRefInfo::NoModRef;
      case SVF::ModRefInfo::Mod:
        return llvm::ModRefInfo::Mod;
      case SVF::ModRefInfo::Ref:
        return llvm::ModRefInfo::Ref;
      case SVF::ModRefInfo::ModRef:
        return llvm::ModRefInfo::ModRef;
      default:
        assert(false && "Unhandled modref info from SVF");
    }
  }
  return llvm::ModRefInfo::ModRef;
#else
  return ModRefInfo::ModRef;
#endif
}

ModRefInfo NoelleSVFIntegration::getModRefInfo(CallBase *i,
                                               const MemoryLocation &loc) {
#ifdef NOELLE_ENABLE_SVF
  if (auto callInst = dyn_cast<CallInst>(i)) {
    SVF::SVFValue *svfVal1 =
        SVF::LLVMModuleSet::getLLVMModuleSet()->getSVFValue(callInst);
    SVF::CallSite callsite = SVF::SVFUtil::getSVFCallSite(svfVal1);
    SVF::CallICFGNode *icfgNode =
        icfg->getCallICFGNode(callsite.getInstruction());
    SVF::SVFValue *svfVal2 =
        SVF::LLVMModuleSet::getLLVMModuleSet()->getSVFValue(loc.Ptr);
    switch (mssa->getMRGenerator()->getModRefInfo(icfgNode, svfVal2)) {
      case SVF::ModRefInfo::NoModRef:
        return llvm::ModRefInfo::NoModRef;
      case SVF::ModRefInfo::Mod:
        return llvm::ModRefInfo::Mod;
      case SVF::ModRefInfo::Ref:
        return llvm::ModRefInfo::Ref;
      case SVF::ModRefInfo::ModRef:
        return llvm::ModRefInfo::ModRef;
      default:
        assert(false && "Unhandled modref info from SVF");
    }
  }
  return llvm::ModRefInfo::ModRef;
#else
  return ModRefInfo::ModRef;
#endif
}

ModRefInfo NoelleSVFIntegration::getModRefInfo(CallBase *i, CallBase *j) {
#ifdef NOELLE_ENABLE_SVF
  auto callInstI = llvm::dyn_cast<llvm::CallInst>(i);
  auto callInstJ = llvm::dyn_cast<llvm::CallInst>(j);
  if (true && (callInstI != nullptr) && (callInstJ != nullptr)) {
    SVF::SVFValue *svfVal1 =
        SVF::LLVMModuleSet::getLLVMModuleSet()->getSVFValue(callInstI);
    SVF::CallSite callsite1 = SVF::SVFUtil::getSVFCallSite(svfVal1);
    SVF::CallICFGNode *icfgNode1 =
        icfg->getCallICFGNode(callsite1.getInstruction());
    SVF::SVFValue *svfVal2 =
        SVF::LLVMModuleSet::getLLVMModuleSet()->getSVFValue(callInstJ);
    SVF::CallSite callsite2 = SVF::SVFUtil::getSVFCallSite(svfVal2);
    SVF::CallICFGNode *icfgNode2 =
        icfg->getCallICFGNode(callsite2.getInstruction());
    switch (mssa->getMRGenerator()->getModRefInfo(icfgNode1, icfgNode2)) {
      case SVF::ModRefInfo::NoModRef:
        return llvm::ModRefInfo::NoModRef;
      case SVF::ModRefInfo::Mod:
        return llvm::ModRefInfo::Mod;
      case SVF::ModRefInfo::Ref:
        return llvm::ModRefInfo::Ref;
      case SVF::ModRefInfo::ModRef:
        return llvm::ModRefInfo::ModRef;
      default:
        assert(false && "Unhandled modref info from SVF");
    }
  }
  return llvm::ModRefInfo::ModRef;
#else
  return ModRefInfo::ModRef;
#endif
}

AliasResult NoelleSVFIntegration::alias(const MemoryLocation &loc1,
                                        const MemoryLocation &loc2) {
#ifdef NOELLE_ENABLE_SVF
  return NoelleSVFIntegration::alias(loc1.Ptr, loc2.Ptr);
#else
  return AliasResult::MayAlias;
#endif
}

AliasResult NoelleSVFIntegration::alias(const Value *v1, const Value *v2) {
#ifdef NOELLE_ENABLE_SVF
  SVF::SVFValue *svfV1 =
      SVF::LLVMModuleSet::getLLVMModuleSet()->getSVFValue(v1);
  SVF::SVFValue *svfV2 =
      SVF::LLVMModuleSet::getLLVMModuleSet()->getSVFValue(v2);
  switch (wpa->alias(svfV1, svfV2)) {
    case SVF::AliasResult::MayAlias:
    case SVF::AliasResult::PartialAlias:
      return llvm::AliasResult::MayAlias;
    case SVF::AliasResult::NoAlias:
      return llvm::AliasResult::NoAlias;
    case SVF::AliasResult::MustAlias:
      return llvm::AliasResult::MustAlias;
    default:
      assert(false && "Unhandled alias result from SVF");
  }
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
