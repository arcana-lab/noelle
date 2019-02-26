#ifndef CAT_ORACLEALIASRESULT_HPP
#define CAT_ORACLEALIASRESULT_HPP

#include <set>
#include <UniqueIRMarker.hpp>


using llvm::Value;

class OracleAliasFunctionResults {
 public:
  OracleAliasFunctionResults() : ReadAfterRead(), ReadAfterWrite(),
                                 WriteAfterWrite(), WriteAfterRead() {}

  using Dependency = std::pair<const Value *, const Value *>;
  using Dependencies = std::set<Dependency>;
  Dependencies ReadAfterWrite;
  Dependencies ReadAfterRead;
  Dependencies WriteAfterRead;
  Dependencies WriteAfterWrite;
};

class OracleAliasResults {
 public:

  using ModuleFunctionAliasResults = std::map<IDType /* ModuleID */,
  std::map<IDType /* FunctionID */,
           OracleAliasFunctionResults>>;

  OracleAliasFunctionResults& getFunctionResults(IDType moduleID, IDType functionID);

  void unionFunctionAlias( OracleAliasResults &res );

  void addFunctionRaW(IDType moduleID, IDType functionID,
                      OracleAliasFunctionResults::Dependency);
  void addFunctionRaR(IDType moduleID, IDType functionID,
                      OracleAliasFunctionResults::Dependency);
  void addFunctionWaR(IDType moduleID, IDType functionID,
                      OracleAliasFunctionResults::Dependency);
  void addFunctionWaW(IDType moduleID, IDType functionID,
                      OracleAliasFunctionResults::Dependency);

 private:
  ModuleFunctionAliasResults res;

};


#endif //CAT_ORACLEALIASRESULT_HPP
