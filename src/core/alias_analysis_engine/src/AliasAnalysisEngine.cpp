#include <assert.h>

#include "noelle/core/AliasAnalysisEngine.hpp"

namespace llvm::noelle {

AliasAnalysisEngine::AliasAnalysisEngine(const std::string &name, void *ptr)
  : n{ name },
    rawPtr{ ptr } {
  assert(rawPtr != nullptr);
  assert(!name.empty());
  return;
}

std::string AliasAnalysisEngine::getName(void) const {
  return this->n;
}

void *AliasAnalysisEngine::getRawPointer(void) const {
  return this->rawPtr;
}

} // namespace llvm::noelle
