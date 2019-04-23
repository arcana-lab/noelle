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
                                 dependencies{ make_pair(ReadAfterWrite, DependencyType::RaW),
                                               make_pair(WriteAfterRead, DependencyType ::WaR),
                                               make_pair(WriteAfterWrite, DependencyType::WaW) }
  { }

  using Dependency = std::pair<const Value *, const Value *>;
  using Dependencies = std::set<Dependency>;

  enum class DependencyType {
    RaR,
    RaW,
    WaR,
    WaW
  };

  Dependencies ReadAfterWrite;
  Dependencies ReadAfterRead;
  Dependencies WriteAfterRead;
  Dependencies WriteAfterWrite;

  std::array<std::pair<Dependencies, DependencyType>, 3> dependencies;
};

class OracleAliasResults {
 public:

  using ModuleFunctionAliasResults = std::map<IDType /* ModuleID */,
  std::map<IDType /* FunctionID */,
       std::map<IDType /* LoopID */,
           OracleAliasFunctionResults>>>;

  optional<reference_wrapper<OracleAliasFunctionResults>> getFunctionResults(IDType moduleID, IDType functionID, IDType loopID);

  void didRecordFunctionDependencies(IDType moduleID, IDType functionID, IDType loopID);

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
