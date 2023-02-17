#pragma once

#include <string>

namespace llvm::noelle {

class AliasAnalysisEngine {
public:
  AliasAnalysisEngine(const std::string &name, void *rawPtr);

  std::string getName(void) const;

  void *getRawPointer(void) const;

protected:
  std::string n;
  void *rawPtr;
};

} // namespace llvm::noelle
