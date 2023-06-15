/*
 * Copyright 2016 - 2023  Angelo Matni, Yian Su, Simone Campanoni
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
#pragma once

#include "noelle/core/SystemHeaders.hpp"
#include "noelle/core/TalkDown.hpp"
#include "noelle/core/AllocAA.hpp"
#include "noelle/core/PDGPrinter.hpp"
#include "noelle/core/DataFlow.hpp"
#include "noelle/core/PDG.hpp"
#include "noelle/core/CallGraph.hpp"
#include "noelle/core/AliasAnalysisEngine.hpp"

namespace llvm::noelle {

enum class PDGVerbosity { Disabled, Minimal, Maximal, MaximalAndPDG };

class PDGAnalysis : public ModulePass {
public:
  static char ID;

  PDGAnalysis();

  virtual ~PDGAnalysis();

  bool doInitialization(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  void releaseMemory() override;

  bool runOnModule(Module &M) override;

  PDG *getPDG(void);

  noelle::CallGraph *getProgramCallGraph(void);

  static bool isTheLibraryFunctionPure(Function *libraryFunction);

  static bool isTheLibraryFunctionThreadSafe(Function *libraryFunction);

  static std::set<AliasAnalysisEngine *> getProgramAliasAnalysisEngines(void);

private:
  Module *M;
  PDG *programDependenceGraph;
  AllocAA *allocAA;
  TalkDown *talkdown;
  DataFlowAnalysis dfa;
  PDGVerbosity verbose;
  bool embedPDG;
  bool dumpPDG;
  bool performThePDGComparison;
  bool disableSVF;
  bool disableAllocAA;
  bool disableRA;
  PDGPrinter printer;
  noelle::CallGraph *noelleCG;

  std::unordered_set<const Function *> internalFuncs;
  std::unordered_set<const Function *> unhandledExternalFuncs;
  std::unordered_map<const Function *, std::unordered_set<const Function *>>
      reachableUnhandledExternalFuncs;

  void initializeSVF(Module &M);
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
      std::function<void(DGEdge<Value> *dependenceMissingInPdg2)> func);

  bool hasPDGAsMetadata(Module &);

  PDG *constructPDGFromMetadata(Module &);
  PDG *constructFunctionDGFromMetadata(Function &);
  void constructNodesFromMetadata(PDG *,
                                  Function &,
                                  unordered_map<MDNode *, Value *> &);
  void constructEdgesFromMetadata(PDG *,
                                  Function &,
                                  unordered_map<MDNode *, Value *> &);
  DGEdge<Value> *constructEdgeFromMetadata(PDG *,
                                           MDNode *,
                                           unordered_map<MDNode *, Value *> &);

  void embedPDGAsMetadata(PDG *);
  void embedNodesAsMetadata(PDG *,
                            LLVMContext &,
                            unordered_map<Value *, MDNode *> &);
  void embedEdgesAsMetadata(PDG *,
                            LLVMContext &,
                            unordered_map<Value *, MDNode *> &);
  MDNode *getEdgeMetadata(DGEdge<Value> *,
                          LLVMContext &,
                          unordered_map<Value *, MDNode *> &);
  MDNode *getSubEdgesMetadata(DGEdge<Value> *,
                              LLVMContext &,
                              unordered_map<Value *, MDNode *> &);

  void trimDGUsingCustomAliasAnalysis(PDG *pdg);

  PDG *constructPDGFromAnalysis(Module &M);
  PDG *constructFunctionDGFromAnalysis(Function &F);
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

  bool edgeIsNotLoopCarriedMemoryDependency(DGEdge<Value> *edge);
  bool isBackedgeOfLoadStoreIntoSameOffsetOfArray(DGEdge<Value> *edge,
                                                  LoadInst *load,
                                                  StoreInst *store);
  bool isBackedgeIntoSameGlobal(DGEdge<Value> *edge);
  bool isMemoryAccessIntoDifferentArrays(DGEdge<Value> *edge);

  bool canPrecedeInCurrentIteration(Instruction *from, Instruction *to);

  bool edgeIsAlongNonMemoryWritingFunctions(DGEdge<Value> *edge);

  bool isInIndependentRegion(Instruction *, Instruction *);

  bool canMemoryEdgeBeRemoved(PDG *pdg, DGEdge<Value> *edge);

  static const StringSet<> externalFuncsHaveNoSideEffectOrHandledBySVF;

  static const StringSet<> externalThreadSafeFunctions;
};

} // namespace llvm::noelle
