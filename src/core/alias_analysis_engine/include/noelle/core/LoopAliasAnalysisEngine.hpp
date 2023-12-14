#ifndef NOELLE_SRC_CORE_ALIAS_ANALYSIS_ENGINE_LOOPALIASANALYSISENGINE_H_
#define NOELLE_SRC_CORE_ALIAS_ANALYSIS_ENGINE_LOOPALIASANALYSISENGINE_H_

#include "noelle/core/AliasAnalysisEngine.hpp"

namespace arcana::noelle {

class LoopAliasAnalysisEngine : public AliasAnalysisEngine {
public:
  LoopAliasAnalysisEngine(const std::string &name, void *rawPtr);

  std::string getName(void) const override;

protected:
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_ALIAS_ANALYSIS_ENGINE_LOOPALIASANALYSISENGINE_H_
