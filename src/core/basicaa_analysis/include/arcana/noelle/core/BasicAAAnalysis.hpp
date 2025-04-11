#ifndef NOELLE_SRC_CORE_BASICAA_ANALYSIS_DEPENDENCE_ANALYSIS_H_
#define NOELLE_SRC_CORE_BASICAA_ANALYSIS_DEPENDENCE_ANALYSIS_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/DependenceAnalysis.hpp"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/Attributes.h"

namespace arcana::noelle {

/*
 * Port the part of LLVM's BasicAliasAnalysis that reads function attributes
 * into Noelle via the custom dependence analysis interface.
 */
class BasicAAAnalysis : public DependenceAnalysis {
public:
  BasicAAAnalysis();

  MemoryDataDependenceStrength isThereThisMemoryDataDependenceType(
      DataDependenceType t,
      Instruction *fromInst,
      Instruction *toInst) override;

  MemoryDataDependenceStrength isThereThisMemoryDataDependenceType(
      DataDependenceType t,
      Instruction *fromInst,
      Instruction *toInst,
      Function &function) override;

  MemoryDataDependenceStrength isThereThisMemoryDataDependenceType(
      DataDependenceType t,
      Instruction *fromInst,
      Instruction *toInst,
      LoopStructure &loop) override;
};

} // namespace arcana::noelle

#endif
