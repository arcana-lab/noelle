#pragma once

#include "SystemHeaders.hpp"

using namespace llvm;

class UniqueIRMarkerPass : public ModulePass {
 public:
  static char ID;

  UniqueIRMarkerPass();
  bool doInitialization (Module &M) override ;
  void getAnalysisUsage(AnalysisUsage& ) const override;
  bool runOnModule(Module& ) override;
};
