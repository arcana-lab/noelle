
#include <OracleAliasResult.hpp>

#include "OracleAliasResult.hpp"





OracleAliasFunctionResults &OracleAliasResults::getFunctionResults(IDType moduleID,
                                                                                         IDType functionID) {
  return res[moduleID][functionID];
}
void OracleAliasResults::addFunctionRaW(IDType moduleID, IDType functionID, OracleAliasFunctionResults::Dependency dep) {
  res[moduleID][functionID].ReadAfterWrite.insert(dep);
}

void OracleAliasResults::addFunctionRaR(IDType moduleID, IDType functionID, OracleAliasFunctionResults::Dependency dep) {
  res[moduleID][functionID].ReadAfterRead.insert(dep);
}

void OracleAliasResults::addFunctionWaR(IDType moduleID, IDType functionID, OracleAliasFunctionResults::Dependency dep) {
  res[moduleID][functionID].WriteAfterRead.insert(dep);
}

void OracleAliasResults::addFunctionWaW(IDType moduleID, IDType functionID, OracleAliasFunctionResults::Dependency dep) {
  res[moduleID][functionID].WriteAfterWrite.insert(dep);
}
void OracleAliasResults::unionFunctionAlias(OracleAliasResults &res) {
  for ( auto &Ms : res.res ) {
    for ( auto &Fs : Ms.second ) {
      {
        auto &FsRaW = Fs.second.ReadAfterWrite;
        this->res[Ms.first][Fs.first].ReadAfterWrite.insert(FsRaW.begin(), FsRaW.end());
      }

      {
        auto &FsRaR = Fs.second.ReadAfterRead;
        this->res[Ms.first][Fs.first].ReadAfterWrite.insert(FsRaR.begin(), FsRaR.end());
      }

      {
        auto &FsWaR = Fs.second.WriteAfterRead;
        this->res[Ms.first][Fs.first].ReadAfterWrite.insert(FsWaR.begin(), FsWaR.end());
      }

      {
        auto &FsWaW = Fs.second.WriteAfterWrite;
        this->res[Ms.first][Fs.first].ReadAfterWrite.insert(FsWaW.begin(), FsWaW.end());
      }
    }
  }
}
