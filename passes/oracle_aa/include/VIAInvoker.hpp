

#ifndef CAT_VIAINVOKER_HPP
#define CAT_VIAINVOKER_HPP

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Value.h>
#include <llvm/Analysis/LoopInfo.h>


#include <map>
#include <set>

#include <UniqueIRMarkerReader.hpp>
#include <IDToValueMapper.hpp>

#include "UniqueIRMarker.hpp"
#include "OracleAliasResult.hpp"

namespace oracle_aa {

using namespace llvm;

class VIAInvoker {
 public:
  VIAInvoker(Module &, ModulePass &);

  void runInference(StringRef inputArgs);

  std::shared_ptr<OracleAliasResults> getResults();

 private:
  void buildOracleDDGConfig(SmallVector<Loop *, 8> lp);
  void dumpModule();
  void executeVIAInference(StringRef inputArgs);
  void parseResponse();

  Module &M;
  ModulePass &MP;
  IDType moduleID;

  std::string viaConfigFilename;
  std::string moduleBitcodeFilename;
  std::string viaResultFilename;

  std::shared_ptr<OracleAliasResults> results;

  unsigned int NumOfLoops = 0;

  const static StringRef RAR;
  const static StringRef RAW;
  const static StringRef WAR;
  const static StringRef WAW;

  Value *getPtrValue(Instruction *I);

  constexpr static char* DependenciesKey = "Dependencies";
};

};

#endif //CAT_VIAINVOKER_HPP
