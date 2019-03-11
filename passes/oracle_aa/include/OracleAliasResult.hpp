#ifndef CAT_ORACLEALIASRESULT_HPP
#define CAT_ORACLEALIASRESULT_HPP

#include <set>
#include <array>
#include <UniqueIRMarker.hpp>


using llvm::Value;

class OracleAliasFunctionResults {
 public:
  OracleAliasFunctionResults() : ReadAfterRead(), ReadAfterWrite(),
                                 WriteAfterWrite(), WriteAfterRead(),
                                 dependencies{ ReadAfterWrite, WriteAfterRead, WriteAfterWrite }{}

  using Dependency = std::pair<const Value *, const Value *>;
  using Dependencies = std::set<Dependency>;
  Dependencies ReadAfterWrite;
  Dependencies ReadAfterRead;
  Dependencies WriteAfterRead;
  Dependencies WriteAfterWrite;

  std::array<Dependencies, 4> dependencies;
};

class OracleAliasResults {
 public:

  using ModuleFunctionAliasResults = std::map<IDType /* ModuleID */,
  std::map<IDType /* FunctionID */,
       std::map<IDType /* LoopID */,
           OracleAliasFunctionResults>>>;

  OracleAliasFunctionResults& getFunctionResults(IDType moduleID, IDType functionID, IDType loopID);

  void unionFunctionAlias( OracleAliasResults &res );

  void addFunctionRaW(IDType moduleID, IDType functionID, IDType loopID,
                      OracleAliasFunctionResults::Dependency);
  void addFunctionRaR(IDType moduleID, IDType functionID, IDType loopID,
                      OracleAliasFunctionResults::Dependency);
  void addFunctionWaR(IDType moduleID, IDType functionID, IDType loopID,
                      OracleAliasFunctionResults::Dependency);
  void addFunctionWaW(IDType moduleID, IDType functionID, IDType loopID,
                      OracleAliasFunctionResults::Dependency);

 private:
  ModuleFunctionAliasResults res;

};


#endif //CAT_ORACLEALIASRESULT_HPP
