#ifndef NOELLE_SRC_CORE_ALIAS_ANALYSIS_ENGINE_ALIASANALYSISENGINE_H_
#define NOELLE_SRC_CORE_ALIAS_ANALYSIS_ENGINE_ALIASANALYSISENGINE_H_
#pragma once

#include "noelle/core/SystemHeaders.hpp"

namespace arcana::noelle {

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

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_ALIAS_ANALYSIS_ENGINE_ALIASANALYSISENGINE_H_
