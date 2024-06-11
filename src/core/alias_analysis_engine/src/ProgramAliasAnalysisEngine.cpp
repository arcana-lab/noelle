#include "arcana/noelle/core/ProgramAliasAnalysisEngine.hpp"

namespace arcana::noelle {

ProgramAliasAnalysisEngine::ProgramAliasAnalysisEngine(const std::string &name,
                                                       void *ptr)
  : AliasAnalysisEngine{ name, ptr } {
  return;
}

std::string ProgramAliasAnalysisEngine::getName(void) const {
  return "ProgramAliasAnalysisEngine \"" + this->n + "\"";
}

} // namespace arcana::noelle
