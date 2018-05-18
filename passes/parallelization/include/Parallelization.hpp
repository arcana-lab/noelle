#pragma once

#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

namespace llvm {
  struct Parallelization : public ModulePass {
    public:
      static char ID;

      Parallelization();
      virtual ~Parallelization();

      bool doInitialization (Module &M) override ;

      void getAnalysisUsage(AnalysisUsage &AU) const override ;

      bool runOnModule (Module &M) override ;

      std::vector<Function *> * getModuleFunctionsReachableFrom (Module *module, Function *startingPoint);

      std::vector<Loop *> * getModuleLoops (void);

      Function * createFunctionForTheLoopBody ();
  };
}
