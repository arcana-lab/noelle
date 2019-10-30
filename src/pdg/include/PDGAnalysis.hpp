/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "WPA/WPAPass.h"

#include "PDG.hpp"
#include "AllocAA.hpp"
#include "PDGPrinter.hpp"

using namespace llvm;

namespace llvm {
  enum class PDGVerbosity { Disabled, Minimal, Maximal };

  struct PDGAnalysis : public ModulePass {
    public:
      static char ID;
      PDGAnalysis();
      virtual ~PDGAnalysis();

      bool doInitialization (Module &M) override ;

      void getAnalysisUsage(AnalysisUsage &AU) const override ;

      bool runOnModule (Module &M) override ;

      PDG * getFunctionPDG (Function &F) ;

      PDG * getPDG () ;

    private:
      Module *M;
      PDG *programDependenceGraph;
      WPAPass *wpa;
      AllocAA *allocAA;
      std::set<Function *> CGUnderMain;
      PDGVerbosity verbose;
      PDGPrinter printer;

      void trimDGUsingCustomAliasAnalysis (PDG *pdg);

      // TODO: Find a way to extract this into a helper module for all passes in the PDG project
      void collectCGUnderFunctionMain (Module &M);

      template <class InstI, class InstJ>
      void addEdgeFromMemoryAlias(PDG *, Function &, AAResults &, InstI *, InstJ *, bool WAW);

      void addEdgeFromFunctionModRef(PDG *, Function &, AAResults &, StoreInst *, CallInst *);
      void addEdgeFromFunctionModRef(PDG *, Function &, AAResults &, LoadInst *, CallInst *);
      void addEdgeFromFunctionModRef(PDG *, Function &, AAResults &, CallInst *, CallInst *);

      void iterateInstForStoreAliases(PDG *, Function &, AAResults &, StoreInst *);
      void iterateInstForLoadAliases(PDG *, Function &, AAResults &, LoadInst *);
      void iterateInstForModRef(PDG *, Function &, AAResults &, CallInst &);

      void constructEdgesFromUseDefs (PDG *pdg);
      void constructEdgesFromAliases (PDG *pdg, Module &M);
      void constructEdgesFromControl (PDG *pdg, Module &M);
      void constructEdgesFromAliasesForFunction (PDG *pdg, Function &F, AAResults &AA);
      void constructEdgesFromControlForFunction (PDG *pdg, Function &F, PostDominatorTree &postDomTree);

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
  };
}
