#include "noelle/core/AliasAnalysisEngine.hpp"

namespace arcana::noelle {

AliasAnalysisEngine::AliasAnalysisEngine(const std::string &name, void *ptr)
  : n{ name },
    rawPtr{ ptr } {
  assert(rawPtr != nullptr);
  assert(!name.empty());
  return;
}

void *AliasAnalysisEngine::getRawPointer(void) const {
  return this->rawPtr;
}

AliasAnalysisEngine::~AliasAnalysisEngine() {
  return;
}

} // namespace arcana::noelle
