#pragma once

#include "noelle/core/SystemHeaders.hpp"

namespace llvm::noelle {

class AliasAnalysisEngine {
public:
  AliasAnalysisEngine(const std::string &name, void *rawPtr);

  void *getRawPointer(void) const;

  virtual std::string getName(void) const = 0;

  virtual ~AliasAnalysisEngine();

protected:
  std::string n;
  void *rawPtr;
};

} // namespace llvm::noelle
