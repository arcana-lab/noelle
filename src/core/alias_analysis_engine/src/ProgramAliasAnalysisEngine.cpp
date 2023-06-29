#include "noelle/core/ProgramAliasAnalysisEngine.hpp"

namespace llvm::noelle {

ProgramAliasAnalysisEngine::ProgramAliasAnalysisEngine(const std::string &name,
                                                       void *ptr)
  : AliasAnalysisEngine{ name, ptr } {
  return;
}

std::string ProgramAliasAnalysisEngine::getName(void) const {
  return "ProgramAliasAnalysisEngine \"" + this->n + "\"";
}

} // namespace llvm::noelle
