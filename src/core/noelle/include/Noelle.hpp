/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "SystemHeaders.hpp"

#include "PDGAnalysis.hpp"
#include "LoopDependenceInfo.hpp"
#include "Queue.hpp"
#include "HotProfiler.hpp"
#include "DataFlow.hpp"
#include "Scheduler.hpp"
#include "StayConnectedNestedLoopForest.hpp"

#include "MemoryAnalysisModules/LoopAA.h"

namespace llvm::noelle {

  enum class Verbosity { Disabled, Minimal, Maximal };

  class Noelle : public ModulePass {
    public:

      /*
       * Fields.
       */
      static char ID;
      IntegerType *int1, *int8, *int16, *int32, *int64;
      Queue queues;

      /*
       * Methods.
       */
      Noelle();

      bool doInitialization (Module &M) override ;

      void getAnalysisUsage(AnalysisUsage &AU) const override ;

      bool runOnModule (Module &M) override ;

      std::vector<LoopDependenceInfo *> * getLoops (void) ;

      std::vector<LoopDependenceInfo *> * getLoops (
        double minimumHotness
      );

      std::vector<LoopDependenceInfo *> * getLoops (
        Function *function
      );

      std::vector<LoopDependenceInfo *> * getLoops (
        Function *function,
        double minimumHotness
      );

      std::vector<LoopStructure *> * getLoopStructures (void) ;

      std::vector<LoopStructure *> * getLoopStructures (
        double minimumHotness
      );

      std::vector<LoopStructure *> * getLoopStructures (
        Function *function
      );

      std::vector<LoopStructure *> * getLoopStructures (
        Function *function,
        double minimumHotness
      );

      LoopDependenceInfo * getLoop (
        LoopStructure *loop
      );

      LoopDependenceInfo * getLoop (
        LoopStructure *loop,
        std::unordered_set<LoopDependenceInfoOptimization> optimizations
      );

      uint32_t getNumberOfProgramLoops (void);

      uint32_t getNumberOfProgramLoops (
        double minimumHotness
        );

      void sortByHotness (
        std::vector<LoopDependenceInfo *> & loops
        ) ;

      void sortByHotness (
        std::vector<LoopStructure *> & loops
        ) ;

      void sortByStaticNumberOfInstructions (
        std::vector<LoopDependenceInfo *> & loops
        ) ;

      StayConnectedNestedLoopForest * organizeLoopsInTheirNestingForest (
        std::vector<LoopStructure *> const & loops
        ) ;

      void filterOutLoops (
        std::vector<LoopStructure *> & loops,
        std::function<bool (LoopStructure *)> filter
        ) ;

      void filterOutLoops (
        StayConnectedNestedLoopForest *f, 
        std::function<bool (LoopStructure *)> filter
        ) ;

      Module * getProgram (void) const ;

      Function * getEntryFunction (void) const ;

      Hot * getProfiles (void) ;

      PDG * getProgramDependenceGraph (void) ;

      PDG * getFunctionDependenceGraph (Function *f) ;

      DataFlowAnalysis getDataFlowAnalyses (void) const ;

      DataFlowEngine getDataFlowEngine (void) const ;

      Scheduler getScheduler (void) const ;

      DominatorSummary * getDominators (Function *f) ;

      CallGraph * getProgramCallGraph (void) ;

      Verbosity getVerbosity (void) const ;

      double getMinimumHotness (void) const ;

      Type * getIntegerType (uint32_t bitwidth) const ;

      Type * getVoidPointerType (void) const ;

      Type * getVoidType (void) const ;

      uint32_t getMaximumNumberOfCores (void) const ;

      uint64_t numberOfProgramInstructions (void) const ;

      /**
       * \brief Check whether a transformation is enabled.
       *
       * This method returns true if the transformation \param transformation is enabled.
       *
       * \param transformation The transformation to consider.
       * \return true if the transformation is enabled. False otherwise.
       */
      bool isTransformationEnabled (Transformation transformation);

      bool shouldLoopsBeHoistToMain (void) const ;

      std::vector<Function *> * getModuleFunctionsReachableFrom (
        Module *module,
        Function *startingPoint
        );

      void linkTransformedLoopToOriginalFunction (
        Module *module,
        BasicBlock *originalPreHeader,
        BasicBlock *startOfParLoopInOriginalFunc,
        BasicBlock *endOfParLoopInOriginalFunc,
        Value *envArray,
        Value *envIndexForExitVariable,
        std::vector<BasicBlock *> &loopExitBlocks
        );

      ~Noelle();

    private:
      Verbosity verbose;
      double minHot;
      Module *program;
      Hot *profiles;
      PDG *programDependenceGraph;
      std::unordered_set<Transformation> enabledTransformations;
      uint32_t maxCores;
      bool hoistLoopsToMain;
      bool loopAwareDependenceAnalysis;
      CallGraph *pcg;
      PDGAnalysis *pdgAnalysis;
      liberty::LoopAA *loopAA;

      char *filterFileName;
      bool hasReadFilterFile;
      std::vector<uint32_t> loopThreads;
      std::vector<uint32_t> techniquesToDisable;
      std::vector<uint32_t> DOALLChunkSize;
      std::unordered_map<BasicBlock *, uint32_t> loopHeaderToLoopIndexMap;

      uint32_t fetchTheNextValue (
        std::stringstream &stream
        );

      bool checkToGetLoopFilteringInfo (void) ;

      LoopDependenceInfo * getLoopDependenceInfoForLoop (
        Loop *loop,
        PDG *functionPDG,
        DominatorSummary *DS,
        ScalarEvolution *SE,
        uint32_t techniquesToDisable,
        uint32_t DOALLChunkSize,
        uint32_t maxCores
      );

      bool isLoopHot (LoopStructure *loopStructure, double minimumHotness) ;
      bool isFunctionHot (Function *function, double minimumHotness) ;

  };

}
