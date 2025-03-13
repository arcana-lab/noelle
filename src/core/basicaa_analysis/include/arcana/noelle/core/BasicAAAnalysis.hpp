#ifndef NOELLE_SRC_CORE_BASICAA_ANALYSIS_DEPENDENCE_ANALYSIS_H_
#define NOELLE_SRC_CORE_BASICAA_ANALYSIS_DEPENDENCE_ANALYSIS_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/DependenceAnalysis.hpp"
#include "llvm/Analysis/TargetLibraryInfo.h"

namespace arcana::noelle {

class MiniBasicAA {
public:
  MiniBasicAA();

  FunctionModRefBehavior getModRefBehavior(const Function *F);

  FunctionModRefBehavior getModRefBehavior(const CallBase *call);

  ModRefInfo getArgModRefInfo(const CallBase *call, unsigned argIdx);
};

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

private:
  /// We use this object to hold functions ported from BasicAA.
  MiniBasicAA miniBasicAA;

  /// These functions are partially ported from LLVM's AliasAnalysis class.
  ///
  /// Their original purpose is to combine the getModRefInfo results from
  /// all LLVM AA algorithms, then improves it with other AA APIs such as
  /// `getModRefBehavior` and `pointsToConstantMemory`, etc.
  ///
  /// In our ported version, we only retained the parts that benefit from
  /// attributes without accessing any other AA algorithm.  Importing other
  /// AAs may improve the result, especially for functions whose attributes
  /// suggest they only access memory via argument pointees.
  ModRefInfo getModRefInfo(const CallBase *call, const MemoryLocation &loc);
  ModRefInfo getModRefInfo(const CallBase *call1, const CallBase *call2);
};

} // namespace arcana::noelle

#endif
