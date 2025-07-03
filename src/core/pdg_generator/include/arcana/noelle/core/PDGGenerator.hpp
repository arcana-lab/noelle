/*
 * Copyright 2016 - 2024  Angelo Matni, Yian Su, Simone Campanoni
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
#ifndef NOELLE_SRC_CORE_PDG_ANALYSIS_PDGGENERATOR_H_
#define NOELLE_SRC_CORE_PDG_ANALYSIS_PDGGENERATOR_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/TalkDown.hpp"
#include "arcana/noelle/core/AllocAA.hpp"
#include "arcana/noelle/core/PDG.hpp"
#include "arcana/noelle/core/PDGPrinter.hpp"
#include "arcana/noelle/core/DataFlow.hpp"
#include "arcana/noelle/core/CallGraph.hpp"
#include "arcana/noelle/core/AliasAnalysisEngine.hpp"
#include "arcana/noelle/core/MayPointsToAnalysis.hpp"
#include "arcana/noelle/core/DependenceAnalysis.hpp"
#include "arcana/noelle/core/CallGraphAnalysis.hpp"

namespace arcana::noelle {

enum class PDGVerbosity { Disabled, Minimal, Maximal, MaximalAndPDG };

class PDGGenerator {
public:
  PDGGenerator(Module &M,
               std::function<llvm::ScalarEvolution &(Function &F)> getSCEV,
               std::function<llvm::LoopInfo &(Function &F)> getLoopInfo,
               std::function<llvm::PostDominatorTree &(Function &F)> getPDT,
               std::function<llvm::CallGraph &(void)> getCallGraph,
               std::function<llvm::AAResults &(Function &F)> getAA,
               bool dumpPDG,
               bool performThePDGComparison,
               bool disableSVF,
               bool disableSVFCallGraph,
               bool disableAllocAA,
               bool disableRA,
               PDGVerbosity verbose);

  void addAnalysis(DependenceAnalysis *a);

  void addAnalysis(CallGraphAnalysis *a);

  void removeAnalysis(DependenceAnalysis *a);

  void removeAnalysis(CallGraphAnalysis *a);

  PDG *getPDG(void);

  noelle::CallGraph *getProgramCallGraph(void);

  void cleanAndEmbedPDGAsMetadata(PDG *pdg);

  void embedSCCAsMetadata(PDG *dg);

  virtual ~PDGGenerator();

  static bool isTheLibraryFunctionPure(Function *libraryFunction);

  static bool isTheLibraryFunctionThreadSafe(Function *libraryFunction);

  static std::set<AliasAnalysisEngine *> getProgramAliasAnalysisEngines(void);

  static bool canAccessMemory(Instruction *i);

  static std::set<const Function *> getFunctionsThatMightEscape(
      Module &currentProgram);

  static std::set<const Function *> getFunctionsWithSignature(
      std::set<const Function *> functions,
      FunctionType *signature);

private:
  Module &M;
  std::function<llvm::ScalarEvolution &(Function &F)> getSCEV;
  std::function<llvm::LoopInfo &(Function &F)> getLoopInfo;
  std::function<llvm::PostDominatorTree &(Function &F)> getPDT;
  std::function<llvm::CallGraph &(void)> getCallGraph;
  std::function<llvm::AAResults &(Function &F)> getAA;
  PDG *programDependenceGraph;
  AllocAA *allocAA;
  MayPointsToAnalysis mpa;
  TalkDown *talkdown;
  DataFlowAnalysis dfa;
  PDGVerbosity verbose;
  bool dumpPDG;
  bool performThePDGComparison;
  bool disableSVF;
  bool disableSVFCallGraph;
  bool disableAllocAA;
  bool disableRA;
  PDGPrinter printer;
  noelle::CallGraph *noelleCG;
  std::set<DependenceAnalysis *> ddAnalyses;
  std::set<CallGraphAnalysis *> cgAnalyses;
  std::unordered_set<const Function *> internalFuncs;
  std::unordered_set<const Function *> unhandledExternalFuncs;
  std::unordered_map<const Function *, std::unordered_set<const Function *>>
      reachableUnhandledExternalFuncs;

  void identifyFunctionsThatInvokeUnhandledLibrary(Module &M);
  void printFunctionReachabilityResult();
  bool isSafeToQueryModRefOfSVF(CallBase *call, BitVector &bv);
  bool isUnhandledExternalFunction(const Function *F);
  bool isInternalFunctionThatReachUnhandledExternalFunction(const Function *F);
  bool cannotReachUnhandledExternalFunction(CallBase *call);
  bool hasNoMemoryOperations(CallBase *call);

  bool comparePDGs(PDG *pdg1, PDG *pdg2);
  bool compareNodes(PDG *pdg1, PDG *pdg2);
  bool compareEdges(PDG *pdg1, PDG *pdg2);
  bool compareEdges(
      PDG *pdg1,
      PDG *pdg2,
      std::function<void(DGEdge<Value, Value> *dependenceMissingInPdg2)> func);

  bool hasPDGAsMetadata(Module &);

  void cleanPDGMetadata();

  PDG *constructPDGFromMetadata(Module &);
  void constructNodesFromMetadata(PDG *,
                                  Function &,
                                  std::unordered_map<MDNode *, Value *> &);
  void constructEdgesFromMetadata(PDG *,
                                  Function &,
                                  std::unordered_map<MDNode *, Value *> &);
  DGEdge<Value, Value> *constructEdgeFromMetadata(
      PDG *,
      MDNode *,
      std::unordered_map<MDNode *, Value *> &);

  void embedPDGAsMetadata(PDG *);
  void embedNodesAsMetadata(PDG *,
                            LLVMContext &,
                            std::unordered_map<Value *, MDNode *> &);
  void embedEdgesAsMetadata(PDG *,
                            LLVMContext &,
                            std::unordered_map<Value *, MDNode *> &);
  MDNode *getEdgeMetadata(DGEdge<Value, Value> *,
                          LLVMContext &,
                          std::unordered_map<Value *, MDNode *> &);
  MDNode *getSubEdgesMetadata(DGEdge<Value, Value> *,
                              LLVMContext &,
                              std::unordered_map<Value *, MDNode *> &);

  void trimDGUsingCustomAliasAnalysis(PDG *pdg);

  PDG *constructPDGFromAnalysis(Module &M);
  void constructEdgesFromUseDefs(PDG *pdg);
  void constructEdgesFromAliases(PDG *pdg, Module &M);
  void constructEdgesFromControl(PDG *pdg, Module &M);
  void constructEdgesFromAliasesForFunction(PDG *pdg, Function &F);
  void constructEdgesFromControlForFunction(PDG *pdg, Function &F);

  void iterateInstForStore(PDG *,
                           Function &,
                           AAResults &,
                           DataFlowResult *,
                           StoreInst *);
  void iterateInstForLoad(PDG *,
                          Function &,
                          AAResults &,
                          DataFlowResult *,
                          LoadInst *);
  void iterateInstForCall(PDG *,
                          Function &,
                          AAResults &,
                          DataFlowResult *,
                          CallBase *);

  void addEdgeFromMemoryAlias(PDG *,
                              Function &,
                              AAResults &,
                              Value *,
                              Value *,
                              DataDependenceType);
  void addEdgeFromFunctionModRef(PDG *,
                                 Function &,
                                 AAResults &,
                                 CallBase *,
                                 StoreInst *,
                                 bool);
  void addEdgeFromFunctionModRef(PDG *,
                                 Function &,
                                 AAResults &,
                                 CallBase *,
                                 LoadInst *,
                                 bool);
  void addEdgeFromFunctionModRef(PDG *,
                                 Function &,
                                 AAResults &,
                                 CallBase *,
                                 CallBase *,
                                 bool);

  void removeEdgesNotUsedByParSchemes(PDG *pdg);

  AliasResult doTheyAlias(PDG *pdg,
                          Function &F,
                          AAResults &AA,
                          Value *instI,
                          Value *instJ);

  bool edgeIsNotLoopCarriedMemoryDependency(DGEdge<Value, Value> *edge);
  bool isBackedgeIntoSameGlobal(DGEdge<Value, Value> *edge);
  bool isMemoryAccessIntoDifferentArrays(DGEdge<Value, Value> *edge);

  bool canPrecedeInCurrentIteration(Instruction *from, Instruction *to);

  bool edgeIsAlongNonMemoryWritingFunctions(DGEdge<Value, Value> *edge);

  bool isInIndependentRegion(Instruction *, Instruction *);

  bool canMemoryEdgeBeRemoved(PDG *pdg, DGEdge<Value, Value> *edge);

  bool canThereBeAMemoryDataDependence(Instruction *fromInst,
                                       Instruction *toInst,
                                       Function &F);

  std::pair<bool, bool> isThereThisMemoryDataDependenceType(
      DataDependenceType t,
      Instruction *fromInst,
      Instruction *toInst,
      Function &F);

  static const StringSet<> externalFuncsHaveNoSideEffectOrHandledBySVF;

  static const StringSet<> externalThreadSafeFunctions;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_PDG_ANALYSIS_PDGGENERATOR_H_
