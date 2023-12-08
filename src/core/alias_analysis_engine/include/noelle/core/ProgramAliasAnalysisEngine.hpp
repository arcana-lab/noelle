#ifndef NOELLE_SRC_CORE_ALIAS_ANALYSIS_ENGINE_PROGRAMALIASANALYSISENGINE_H_
#define NOELLE_SRC_CORE_ALIAS_ANALYSIS_ENGINE_PROGRAMALIASANALYSISENGINE_H_
#pragma once

#include "noelle/core/AliasAnalysisEngine.hpp"

namespace arcana::noelle {

class ProgramAliasAnalysisEngine : public AliasAnalysisEngine {
public:
  ProgramAliasAnalysisEngine(const std::string &name, void *rawPtr);

  std::string getName(void) const override;

protected:
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_ALIAS_ANALYSIS_ENGINE_PROGRAMALIASANALYSISENGINE_H_
