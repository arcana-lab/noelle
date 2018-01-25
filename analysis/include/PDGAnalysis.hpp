#pragma once

using namespace llvm;

namespace llvm {
  struct PDGAnalysis : public ModulePass {
    static char ID;

    PDGAnalysis() : ModulePass{ID} {
    }

    bool doInitialization (Module &M) override ;

    void getAnalysisUsage(AnalysisUsage &AU) const override ;

    bool runOnModule (Module &M) override ;
  };
}
