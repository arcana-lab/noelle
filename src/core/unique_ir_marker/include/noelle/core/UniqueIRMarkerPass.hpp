#ifndef NOELLE_SRC_CORE_UNIQUE_IR_MARKER_UNIQUEIRMARKERPASS_H_
#define NOELLE_SRC_CORE_UNIQUE_IR_MARKER_UNIQUEIRMARKERPASS_H_

#include "noelle/core/SystemHeaders.hpp"

using namespace llvm;

namespace arcana::noelle {

class UniqueIRMarkerPass : public ModulePass {
public:
  static char ID;

  UniqueIRMarkerPass();
  bool doInitialization(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &) const override;
  bool runOnModule(Module &) override;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_UNIQUE_IR_MARKER_UNIQUEIRMARKERPASS_H_
