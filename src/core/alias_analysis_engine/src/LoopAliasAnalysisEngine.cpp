#include "noelle/core/LoopAliasAnalysisEngine.hpp"

namespace llvm::noelle {

LoopAliasAnalysisEngine::LoopAliasAnalysisEngine(const std::string &name,
                                                 void *ptr)
  : AliasAnalysisEngine{ name, ptr } {
  return;
}

std::string LoopAliasAnalysisEngine::getName(void) const {
  return "LoopAliasAnalysisEngine \"" + this->n + "\"";
}

} // namespace llvm::noelle
