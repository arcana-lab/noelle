#pragma once

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Pass.h>

using namespace llvm;

class UniqueIRMarkerPass : public ModulePass {
 public:
  static char ID;

  UniqueIRMarkerPass();
  bool doInitialization (Module &M) override ;
  void getAnalysisUsage(AnalysisUsage& ) const override;
  bool runOnModule(Module& ) override;
};