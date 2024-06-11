/*
 * Copyright 2016 - 2024  Angelo Matni, Simone Campanoni
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
#ifndef NOELLE_H_
#define NOELLE_H_

#include "arcana/noelle/core/Dominators.hpp"
#include "arcana/noelle/core/LoopNestingGraph.hpp"
#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/Queue.hpp"
#include "arcana/noelle/core/LoopForest.hpp"
#include "arcana/noelle/core/PDGGenerator.hpp"
#include "arcana/noelle/core/LDGGenerator.hpp"
#include "noelle/core/DataFlow.hpp"
#include "arcana/noelle/core/LoopContent.hpp"
#include "arcana/noelle/core/HotProfiler.hpp"
#include "arcana/noelle/core/Scheduler.hpp"
#include "arcana/noelle/core/MetadataManager.hpp"
#include "arcana/noelle/core/LoopTransformer.hpp"
#include "arcana/noelle/core/FunctionsManager.hpp"
#include "arcana/noelle/core/GlobalsManager.hpp"
#include "arcana/noelle/core/TypesManager.hpp"
#include "arcana/noelle/core/ConstantsManager.hpp"
#include "arcana/noelle/core/CompilationOptionsManager.hpp"
#include "arcana/noelle/core/CFGAnalysis.hpp"
#include "arcana/noelle/core/CFGTransformer.hpp"
#include "arcana/noelle/core/Linker.hpp"
#include "arcana/noelle/core/AliasAnalysisEngine.hpp"
#include "arcana/noelle/core/MayPointsToAnalysis.hpp"
#include "arcana/noelle/core/DependenceAnalysis.hpp"
#include "arcana/noelle/core/CallGraphAnalysis.hpp"

namespace arcana::noelle {

enum class Verbosity { Disabled, Minimal, Maximal };

class Noelle : public ModulePass {
public:
  /*
   * Methods.
   */
  Noelle();

  bool doInitialization(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  bool runOnModule(Module &M) override;

  FunctionsManager *getFunctionsManager(void);

  GlobalsManager *getGlobalsManager(void);

  CompilationOptionsManager *getCompilationOptionsManager(void);

  TypesManager *getTypesManager(void);

  ConstantsManager *getConstantsManager(void);

  MetadataManager *getMetadataManager(void);

  Linker *getLinker(void);

  std::set<AliasAnalysisEngine *> getAliasAnalysisEngines(void);

  LoopNestingGraph *getLoopNestingGraphForProgram(void);

  LoopForest *getLoopNestingForest(void);

  std::vector<LoopStructure *> *getLoopStructures(void);

  std::vector<LoopStructure *> *getLoopStructures(double minimumHotness);

  std::vector<LoopStructure *> *getLoopStructures(
      double minimumHotness,
      const std::set<Function *> &functions);

  /*
   * @includeLoop return true if the loop given as input must be included (if
   * hot enough). Return false if you want to use the default inclusion policy
   * (INDEX_FILE).
   */
  std::vector<LoopStructure *> *getLoopStructures(
      double minimumHotness,
      std::function<bool(LoopStructure *)> includeLoop);

  /*
   * @includeLoop return true if the loop given as input must be included (if
   * hot enough). Return false if you want to use the default inclusion policy
   * (INDEX_FILE).
   */
  std::vector<LoopStructure *> *getLoopStructures(
      double minimumHotness,
      const std::set<Function *> &functions,
      std::function<std::vector<Function *>(std::set<Function *> functions)>
          orderOfFunctionsToFollow,
      std::function<bool(LoopStructure *)> includeLoop);

  std::vector<LoopStructure *> *getLoopStructures(Function *function);

  std::vector<LoopStructure *> *getLoopStructures(Function *function,
                                                  double minimumHotness);

  std::vector<LoopStructure *> *getLoopStructuresReachableFromEntryFunction(
      void);

  std::vector<LoopStructure *> *getLoopStructuresReachableFromEntryFunction(
      double minimumHotness);

  /*
   * @includeLoop return true if the loop given as input must be included (if
   * hot enough). Return false if you want to use the default inclusion policy
   * (INDEX_FILE).
   */
  std::vector<LoopStructure *> *getLoopStructuresReachableFromEntryFunction(
      double minimumHotness,
      std::function<bool(LoopStructure *)> includeLoop);

  std::vector<LoopContent *> *getLoopContents(void);

  std::vector<LoopContent *> *getLoopContents(double minimumHotness);

  std::vector<LoopContent *> *getLoopContents(Function *function);

  std::vector<LoopContent *> *getLoopContents(Function *function,
                                              double minimumHotness);

  LoopContent *getLoopContent(LoopStructure *loop);

  LoopContent *getLoopContent(
      LoopStructure *loop,
      std::unordered_set<LoopContentOptimization> optimizations);

  LoopContent *getLoopContent(BasicBlock *header,
                              PDG *functionPDG,
                              LoopTransformationsManager *ltm,
                              bool enableLoopAwareDependenceAnalysis = true);

  uint32_t getNumberOfProgramLoops(void);

  uint32_t getNumberOfProgramLoops(double minimumHotness);

  void sortByHotness(std::vector<LoopContent *> &loops);

  void sortByHotness(std::vector<LoopStructure *> &loops);

  std::vector<LoopTree *> sortByHotness(
      const std::unordered_set<LoopTree *> &loops);

  std::vector<SCC *> sortByHotness(const std::set<SCC *> &SCCs);

  void sortByStaticNumberOfInstructions(std::vector<LoopContent *> &loops);

  LoopForest *organizeLoopsInTheirNestingForest(
      std::vector<LoopStructure *> const &loops);

  void filterOutLoops(std::vector<LoopStructure *> &loops,
                      std::function<bool(LoopStructure *)> filter);

  void filterOutLoops(LoopForest *f,
                      std::function<bool(LoopStructure *)> filter);

  Module *getProgram(void) const;

  LLVMContext &getProgramContext(void) const;

  Hot *getProfiles(void);

  void addAnalysis(DependenceAnalysis *a);

  void addAnalysis(CallGraphAnalysis *a);

  PDG *getProgramDependenceGraph(void);

  DataFlowAnalysis getDataFlowAnalyses(void) const;

  CFGAnalysis getCFGAnalysis(void) const;

  CFGTransformer getCFGTransformer(void) const;

  DataFlowEngine getDataFlowEngine(void) const;

  Scheduler getScheduler(void) const;

  MayPointsToAnalysis getMayPointsToAnalysis(void) const;

  LoopTransformer &getLoopTransformer(void);

  DominatorSummary *getDominators(Function *f);

  Verbosity getVerbosity(void) const;

  double getMinimumHotness(void) const;

  uint64_t numberOfProgramInstructions(void) const;

  /**
   * \brief Check whether a transformation is enabled.
   *
   * This method returns true if the transformation \param transformation is
   * enabled.
   *
   * \param transformation The transformation to consider.
   * \return true if the transformation is enabled. False otherwise.
   */
  bool isTransformationEnabled(Transformation transformation);

  bool verifyCode(void) const;

  ~Noelle();

  /*
   * Fields.
   */
  static char ID;
  Queue queues;

private:
  Verbosity verbose;
  double minHot;
  Module *program;
  Hot *profiles;
  PDG *programDependenceGraph;
  std::unordered_set<Transformation> enabledTransformations;
  bool hoistLoopsToMain;
  bool loopAwareDependenceAnalysis;
  PDGGenerator *pdgAnalysis;
  LDGGenerator ldgAnalysis;
  char *filterFileName;
  bool hasReadFilterFile;
  std::map<uint32_t, uint32_t> loopThreads;
  std::map<uint32_t, uint32_t> techniquesToDisable;
  std::map<uint32_t, uint32_t> DOALLChunkSize;
  FunctionsManager *fm;
  GlobalsManager *gm;
  TypesManager *tm;
  ConstantsManager *cm;
  CompilationOptionsManager *om;
  MetadataManager *mm;
  Linker *linker;
  std::set<AliasAnalysisEngine *> aaEngines;

  PDG *getFunctionDependenceGraph(Function *f);

  uint32_t fetchTheNextValue(std::stringstream &stream);

  bool checkToGetLoopFilteringInfo(void);

  LoopContent *getLoopContentForLoop(
      BasicBlock *header,
      PDG *functionPDG,
      DominatorSummary *DS,
      uint32_t techniquesToDisable,
      uint32_t DOALLChunkSize,
      uint32_t maxCores,
      std::unordered_set<LoopContentOptimization> optimizations);

  LoopContent *getLoopContentForLoop(
      LoopTree *loopNode,
      Loop *loop,
      PDG *functionPDG,
      DominatorSummary *DS,
      ScalarEvolution *SE,
      uint32_t techniquesToDisable,
      uint32_t DOALLChunkSize,
      uint32_t maxCores,
      std::unordered_set<LoopContentOptimization> optimizations);

  bool isLoopHot(LoopStructure *loopStructure, double minimumHotness);
  bool isFunctionHot(Function *function, double minimumHotness);

  std::function<std::vector<Function *>(std::set<Function *> fns)>
  fetchFunctionsSorting(void);
};

} // namespace arcana::noelle

#endif // NOELLE_H_
