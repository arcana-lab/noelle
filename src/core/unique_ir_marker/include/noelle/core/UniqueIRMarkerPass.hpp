#ifndef NOELLE_SRC_CORE_UNQIUEIRMARKERPASS_H
#define NOELLE_SRC_CORE_UNQIUEIRMARKERPASS_H
#pragma once

#include "noelle/core/SystemHeaders.hpp"

using namespace llvm;

namespace llvm::noelle {

class UniqueIRMarkerPass : public ModulePass {
public:
  static char ID;

  UniqueIRMarkerPass();
  bool doInitialization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &) const override;
  bool runOnModule(Module &) override;
};

} // namespace llvm::noelle

#endif
