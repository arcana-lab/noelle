
#include <OracleAliasResult.hpp>

#include "OracleAliasResult.hpp"





optional<reference_wrapper<OracleAliasFunctionResults>> OracleAliasResults::getFunctionResults(IDType moduleID, IDType functionID, IDType loopID) {
  auto modIter = res.find(moduleID);
  if (modIter != res.end()) {
    auto &funRes = modIter->second;
    auto funIter = funRes.find(functionID);
    if (funIter != funRes.end()) {
      auto loopRes = funIter->second;
      auto loopIter = loopRes.find(loopID);
      if (loopIter != loopRes.end()) {
        return loopIter->second;
      }
    }
  }
  return nullopt;
}

void OracleAliasResults::addFunctionRaW(IDType moduleID, IDType functionID, IDType loopID, OracleAliasFunctionResults::Dependency dep) {
  res.at(moduleID).at(functionID).at(loopID).ReadAfterWrite.insert(dep);
}

void OracleAliasResults::addFunctionRaR(IDType moduleID, IDType functionID, IDType loopID, OracleAliasFunctionResults::Dependency dep) {
  res.at(moduleID).at(functionID).at(loopID).ReadAfterRead.insert(dep);
}

void OracleAliasResults::addFunctionWaR(IDType moduleID, IDType functionID, IDType loopID, OracleAliasFunctionResults::Dependency dep) {
  res.at(moduleID).at(functionID).at(loopID).WriteAfterRead.insert(dep);
}

void OracleAliasResults::addFunctionWaW(IDType moduleID, IDType functionID, IDType loopID, OracleAliasFunctionResults::Dependency dep) {
  res.at(moduleID).at(functionID).at(loopID).WriteAfterWrite.insert(dep);
}
void OracleAliasResults::unionFunctionAlias(OracleAliasResults &other) {
  for ( auto &Ms : other.res ) {
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
void OracleAliasResults::didRecordFunctionDependencies(IDType moduleID, IDType functionID, IDType loopID) {
  res[moduleID][functionID][loopID];
}
