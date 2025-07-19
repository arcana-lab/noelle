#pragma once

#include <string>
#include <vector>

#include "arcana/noelle/core/Noelle.hpp"
#include "arcana/noelle/core/Lumberjack.hpp"

namespace arcana::noelle {

class ProfilerPass : public llvm::ModulePass {
public:
  static char ID;

  ProfilerPass();
  bool doInitialization(llvm::Module &M) override;
  bool runOnModule(llvm::Module &M) override;
  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;

private:
  noelle::Logger log;
  bool enabled;
};

} // namespace arcana::noelle
