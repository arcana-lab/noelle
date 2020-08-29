/*
 * Copyright 2016 - 2020  Angelo Matni, Yian Su, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "MemoryModel/PointerAnalysis.h"
#include "Util/PTACallGraph.h"
#include "MSSA/MemSSA.h"

#include "SystemHeaders.hpp"
#include "PDG.hpp"
#include "AllocAA.hpp"
#include "PDGPrinter.hpp"
#include "TalkDown.hpp"
#include "DataFlow.hpp"
#include "CallGraph.hpp"

using namespace llvm;

namespace llvm {
  enum class PDGVerbosity { Disabled, Minimal, Maximal, MaximalAndPDG };

  class PDGAnalysis : public ModulePass {
    public:
      static char ID;

      PDGAnalysis();

      virtual ~PDGAnalysis();

      bool doInitialization (Module &M) override ;

      void getAnalysisUsage(AnalysisUsage &AU) const override ;

      void releaseMemory () override ;

      bool runOnModule (Module &M) override ;

      PDG * getFunctionPDG (Function &F) ;

      PDG * getPDG (void) ;

      noelle::CallGraph * getProgramCallGraph (void);

    private:
      Module *M;
      PDG *programDependenceGraph;
      std::unordered_map<Function *, PDG *> functionToFDGMap;
      AllocAA *allocAA;
      std::set<Function *> CGUnderMain;
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
      PointerAnalysis *pta;
      PTACallGraph *callGraph;
      MemSSA *mssa;

      std::unordered_set<const Function *> internalFuncs;
      std::unordered_set<const Function *> unhandledExternalFuncs;
      std::unordered_map<const Function *, std::unordered_set<const Function *>> reachableUnhandledExternalFuncs;
      
      void initializeSVF(Module &M);
      void identifyFunctionsThatInvokeUnhandledLibrary(Module &M);
      void printFunctionReachabilityResult();
      bool isSafeToQueryModRefOfSVF(CallInst *call, BitVector &bv);
      bool isUnhandledExternalFunction(const Function *F);
      bool isInternalFunctionThatReachUnhandledExternalFunction(const Function *F);
      bool cannotReachUnhandledExternalFunction(CallInst *call);
      bool hasNoMemoryOperations(CallInst *call);

      bool comparePDGs(PDG *, PDG *);
      bool compareNodes(PDG *, PDG *);
      bool compareEdges(PDG *, PDG *);

      bool hasPDGAsMetadata(Module &);

      PDG * constructPDGFromMetadata(Module &);
      PDG * constructFunctionDGFromMetadata(Function &);
      void constructNodesFromMetadata(PDG *, Function &, unordered_map<MDNode *, Value *> &);
      void constructEdgesFromMetadata(PDG *, Function &, unordered_map<MDNode *, Value *> &);
      DGEdge<Value> * constructEdgeFromMetadata(PDG *, MDNode *, unordered_map<MDNode *, Value *> &);

      void embedPDGAsMetadata(PDG *);
      void embedNodesAsMetadata(PDG *, LLVMContext &, unordered_map<Value *, MDNode *> &);
      void embedEdgesAsMetadata(PDG *, LLVMContext &, unordered_map<Value *, MDNode *> &);
      MDNode * getEdgeMetadata(DGEdge<Value> *, LLVMContext &, unordered_map<Value *, MDNode *> &);
      MDNode * getSubEdgesMetadata(DGEdge<Value> *, LLVMContext &, unordered_map<Value *, MDNode *> &);

      void trimDGUsingCustomAliasAnalysis (PDG *pdg);

      // TODO: Find a way to extract this into a helper module for all passes in the PDG project
      void collectCGUnderFunctionMain (Module &M);

      PDG * constructPDGFromAnalysis(Module &M);
      PDG * constructFunctionDGFromAnalysis(Function &F);
      void constructEdgesFromUseDefs (PDG *pdg);
      void constructEdgesFromAliases (PDG *pdg, Module &M);
      void constructEdgesFromControl (PDG *pdg, Module &M);
      void constructEdgesFromAliasesForFunction (PDG *pdg, Function &F);
      void constructEdgesFromControlForFunction (PDG *pdg, Function &F);

      void iterateInstForStore(PDG *, Function &, AAResults &, DataFlowResult *, StoreInst *);
      void iterateInstForLoad(PDG *, Function &, AAResults &, DataFlowResult *, LoadInst *);
      void iterateInstForCall(PDG *, Function &, AAResults &, DataFlowResult *, CallInst *);
      
      template<class InstI, class InstJ>
      void addEdgeFromMemoryAlias(PDG *, Function &, AAResults &, InstI *, InstJ *, DataDependenceType);
      void addEdgeFromFunctionModRef(PDG *, Function &, AAResults &, CallInst *, StoreInst *, bool);
      void addEdgeFromFunctionModRef(PDG *, Function &, AAResults &, CallInst *, LoadInst *, bool);
      void addEdgeFromFunctionModRef(PDG *, Function &, AAResults &, CallInst *, CallInst *);

      void removeEdgesNotUsedByParSchemes (PDG *pdg);

      bool edgeIsNotLoopCarriedMemoryDependency (DGEdge<Value> *edge);
      bool isBackedgeOfLoadStoreIntoSameOffsetOfArray (
        DGEdge<Value> *edge,
        LoadInst *load,
        StoreInst *store
      );
      bool isBackedgeIntoSameGlobal (DGEdge<Value> *edge);
      bool isMemoryAccessIntoDifferentArrays (DGEdge<Value> *edge);

      bool canPrecedeInCurrentIteration (Instruction *from, Instruction *to);

      bool edgeIsAlongNonMemoryWritingFunctions (DGEdge<Value> *edge);

      bool isInIndependentRegion(Instruction *, Instruction *);

      // http://www.cplusplus.com/reference/clibrary/ and https://github.com/SVF-tools/SVF/blob/master/lib/Util/ExtAPI.cpp
      const StringSet<> externalFuncsHaveNoSideEffectOrHandledBySVF {
        // ctype.h
        "isalnum",
        "isalpha",
        "isblank",
        "iscntrl",
        "isdigit",
        "isgraph",
        "islower",
        "isprint",
        "ispunct",
        "isspace",
        "isupper",
        "isxdigit",
        "tolower",
        "toupper",

        // math.h
        "cos",
        "sin",
        "tan",
        "acos",
        "asin",
        "atan",
        "atan2",
        "cosh",
        "sinh",
        "tanh",
        "acosh",
        "asinh",
        "atanh",
        "exp",
        "ldexp",
        "log",
        "log10",
        "exp2",
        "expm1",
        "ilogb",
        "log1p",
        "log2",
        "logb",
        "scalbn",
        "scalbln",
        "pow",
        "sqrt",
        "cbrt",
        "hypot",
        "erf",
        "erfc",
        "tgamma",
        "lgamma",
        "ceil",
        "floor",
        "fmod",
        "trunc",
        "round",
        "lround",
        "llround",
        "nearbyint",
        "remainder",
        "copysign",
        "nextafter",
        "nexttoward",
        "fdim",
        "fmax",
        "fmin",
        "fabs",
        "abs",
        "fma",
        "fpclassify",
        "isfinite",
        "isinf",
        "isnan",
        "isnormal",
        "signbit",
        "isgreater",
        "isgreaterequal",
        "isless",
        "islessequal",
        "islessgreater",
        "isunordered",

        // stdlib.h
        "rand",
        "srand",
        
        // time.h
        "clock",
        "difftime",

        // wctype.h
        "iswalnum",
        "iswalpha",
        "iswblank",
        "iswcntrl",
        "iswdigit",
        "iswgraph",
        "iswlower",
        "iswprint",
        "iswpunct",
        "iswspace",
        "iswupper",
        "iswxdigit",
        "towlower",
        "towupper",
        "iswctype",
        "towctrans"
      };
  };
}
