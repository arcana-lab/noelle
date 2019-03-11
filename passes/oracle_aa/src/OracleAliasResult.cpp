
#include <OracleAliasResult.hpp>

#include "OracleAliasResult.hpp"





OracleAliasFunctionResults &OracleAliasResults::getFunctionResults(IDType moduleID, IDType functionID, IDType loopID) {
  return res[moduleID][functionID][loopID];
}
void OracleAliasResults::addFunctionRaW(IDType moduleID, IDType functionID, IDType loopID, OracleAliasFunctionResults::Dependency dep) {
  res[moduleID][functionID][loopID].ReadAfterWrite.insert(dep);
}

void OracleAliasResults::addFunctionRaR(IDType moduleID, IDType functionID, IDType loopID, OracleAliasFunctionResults::Dependency dep) {
  res[moduleID][functionID][loopID].ReadAfterRead.insert(dep);
}

void OracleAliasResults::addFunctionWaR(IDType moduleID, IDType functionID, IDType loopID, OracleAliasFunctionResults::Dependency dep) {
  res[moduleID][functionID][loopID].WriteAfterRead.insert(dep);
}

void OracleAliasResults::addFunctionWaW(IDType moduleID, IDType functionID, IDType loopID, OracleAliasFunctionResults::Dependency dep) {
  res[moduleID][functionID][loopID].WriteAfterWrite.insert(dep);
}
void OracleAliasResults::unionFunctionAlias(OracleAliasResults &res) {
  for ( auto &Ms : res.res ) {
    for (auto &Fs : Ms.second) {
      for (auto &Ls : Fs.second ) {
        {
          auto &FsRaW = Ls.second.ReadAfterWrite;
          this->res[Ms.first][Fs.first][Ls.first].ReadAfterWrite.insert(FsRaW.begin(), FsRaW.end());
        }

        {
          auto &FsRaR = Ls.second.ReadAfterRead;
          this->res[Ms.first][Fs.first][Ls.first].ReadAfterRead.insert(FsRaR.begin(), FsRaR.end());
        }

        {
          auto &FsWaR = Ls.second.WriteAfterRead;
          this->res[Ms.first][Fs.first][Ls.first].WriteAfterRead.insert(FsWaR.begin(), FsWaR.end());
        }

        {
          auto &FsWaW = Ls.second.WriteAfterWrite;
          this->res[Ms.first][Fs.first][Ls.first].WriteAfterWrite.insert(FsWaW.begin(), FsWaW.end());
        }
      }
    }
  }
}
