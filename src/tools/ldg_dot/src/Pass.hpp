#pragma once

#include <string>
#include <vector>

#include "arcana/noelle/core/Noelle.hpp"
#include "arcana/noelle/core/Lumberjack.hpp"

namespace arcana::noelle {

class DotPass : public llvm::ModulePass {
public:
  static char ID;

  DotPass();
  bool doInitialization(llvm::Module &M) override;
  bool runOnModule(llvm::Module &M) override;
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
  void runOnLoop(noelle::Noelle &noelle, noelle::LoopStructure *LS);

private:
  noelle::Logger log;
};

} // namespace arcana::noelle
