
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Constants.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/BitcodeWriter.h>


#include <fstream>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>

#include "VIAInvoker.hpp"
#include "UniqueIRMarkerReader.hpp"

cl::opt<std::string> OracleExecutionScriptPath("exe-script",
    cl::desc("Path to the script used to instrument and execute the current IR"),
    cl::value_desc("filename"), cl::ValueRequired);

cl::opt<std::string> OracleConfigFilepath("config-path",
                                               cl::desc("Path to the oracle config used to specify which loops to "
                                                        "instrument"),
                                               cl::value_desc("filename"), cl::ValueOptional);

using namespace oracle_aa;

// RAR
const StringRef VIAInvoker::RAR = "RAR";

// RAW
const StringRef VIAInvoker::RAW = "RAW";
const StringRef VIAInvoker::RAA = "RAA";

// WAR

const StringRef VIAInvoker::WAR = "WAR";
const StringRef VIAInvoker::FAR = "FAR";

// WAW
const StringRef VIAInvoker::WAW = "WAW";
const StringRef VIAInvoker::WAA = "WAA";
const StringRef VIAInvoker::FAA = "FAA";
const StringRef VIAInvoker::FAW = "FAW";


bool VIAInvoker::isRaR(StringRef R) {
  return R == RAR;
}

bool VIAInvoker::isRaW(StringRef R) {
  return R == RAW || R == RAA;
}

bool VIAInvoker::isWaR(StringRef R) {
  return  R == WAR || R == FAR;
}

bool VIAInvoker::isWaW(StringRef R) {
  return R == WAW || R == WAA || R == FAA || R == FAW;
}


oracle_aa::VIAInvoker::VIAInvoker(Module &M, ModulePass& MP) : M(M), MP(MP) {
  auto m = UniqueIRMarkerReader::getModuleID(&M);
  assert(m && "Must have a module ID, maybe Unique IR Marker has not been run?");
  moduleID = m.value();
  auto modIDStr = std::to_string(moduleID);

  results = std::make_shared<OracleAliasResults>();

  if(OracleConfigFilepath.getNumOccurrences() > 0) {
     viaConfigFilename = OracleConfigFilepath.getValue();
  } else {
     viaConfigFilename = modIDStr + "-oracle-ddg.viaconf";
  }
  moduleBitcodeFilename = modIDStr + ".bc";
  viaResultFilename = modIDStr + "-oracle-ddg.dep";
}

void oracle_aa::VIAInvoker::runInference(StringRef inputArgs) {
  std::ifstream depFile(viaResultFilename);
  if(!depFile.good()) {
    SmallVector<Loop *, 8> sm{};
    buildOracleDDGConfig(sm);
    dumpModule();
    executeVIAInference(inputArgs);
  }
  depFile.close();
  parseResponse();
}

// build a viaconf file for each loop in the program and write it to viaConfigFilename
void oracle_aa::VIAInvoker::buildOracleDDGConfig(llvm::SmallVector<llvm::Loop *, 8> lp) {
  if (OracleConfigFilepath.getNumOccurrences() > 0) {
    std::ifstream depFile(viaConfigFilename);
    if (!depFile.good()) {
      errs() << OracleConfigFilepath << '\n';
      assert( 0 && "Filename passes via -config-path must exist");
    }
    depFile.close();
    return;
  }

  std::vector<uint64_t> loopIDs{};
  for ( auto &F: M ) {
    if (F.empty()) continue;
    auto &LI = MP.getAnalysis<LoopInfoWrapperPass>(F).getLoopInfo();
    for ( auto &L : LI ) {
      // skip inner loops
      if ( L->getLoopDepth() > 1 ) continue;
      auto LID = UniqueIRMarkerReader::getLoopID(L);
      if (LID) {
        loopIDs.push_back(LID.value());
      }
    }
  }

  NumOfLoops = loopIDs.size();

  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> prettyBuffer(buffer);


  prettyBuffer.StartObject();
    prettyBuffer.Key("Monitor"); prettyBuffer.String("Basic");
    prettyBuffer.Key("Model"); prettyBuffer.String("OracleDDG");
    prettyBuffer.Key("Loop");
      prettyBuffer.StartArray();
      for ( auto &LID : loopIDs) {
        prettyBuffer.StartObject();
        prettyBuffer.Key("ModuleID"); prettyBuffer.Uint64(moduleID);
        prettyBuffer.Key("LoopID");   prettyBuffer.Uint64(LID);
        prettyBuffer.EndObject();
      }
      prettyBuffer.EndArray();
  prettyBuffer.EndObject();

  std::ofstream ofs(viaConfigFilename);

  ofs << buffer.GetString() << std::endl;

}

// write the current IR module with (unique Value ids) to a bitcode file
// in the same dir the viaconf file.
void oracle_aa::VIAInvoker::dumpModule() {

  std::error_code EC;

  llvm::raw_fd_ostream OS(moduleBitcodeFilename, EC, sys::fs::OpenFlags::F_None);
  WriteBitcodeToFile(&M, OS);
  OS.flush();

  if(EC) {
    errs() << EC.message() << "\n";
    assert(0 && "EC Failed");
  }
}

// run opt (from a bash script) to instrument the saved module.
// then create an executable from this
// execute the executable
void oracle_aa::VIAInvoker::executeVIAInference(StringRef inputArgs) {

  auto modIDStr = std::to_string(moduleID);
  auto ScriptWithArgs = OracleExecutionScriptPath.getValue() + " " + modIDStr + " " + inputArgs.str();

  // Tell VIA where the viaconf setting is.
  setenv("VIACONF_SETTING", viaConfigFilename.c_str(), 1);

  errs() << ScriptWithArgs << '\n';

  auto returnValue = system(ScriptWithArgs.c_str());
  if (returnValue) {
    errs() << "Return Value: " << returnValue << '\n';
    assert(0 && "Cannot continue with pass since the execution of the script failed");
  }
}

// read the result from the executable which is of the form <moduleID>-oracle-ddg.dep
// create a OracleAliasResult (a quad of all types of memory dependencies).
void oracle_aa::VIAInvoker::parseResponse() {
  // No Loops to consider.
  if (NumOfLoops <= 0) return;

  errs() << viaResultFilename << '\n';
  std::ifstream ifs(viaResultFilename);

  if( !ifs.good() ) {
    assert(0 && "ifs not good");
  }

  rapidjson::Document doc;
  rapidjson::IStreamWrapper isw(ifs);
  doc.ParseStream(isw);

  // Find all Dependency IDs
  std::set<IDType> ids;
  if (doc.IsObject()) {
    auto &resultList = doc["Result"];
    for (auto &iter : resultList.GetArray()) {
      for (auto &dep : iter[DependenciesKey].GetArray()) {
        auto first = dep[1].GetUint64();
        auto second = dep[2].GetUint64();
        ids.insert(first);
        ids.insert(second);
      }
    }

    // Create an ID to Dependency Mapping.
    IDToInstructionMapper idToValueMapper(M);
    auto mapping = idToValueMapper.idToValueMap(ids);

    for (auto &iter : resultList.GetArray()) {
      auto ModuleID = iter["ModuleID"].GetUint64();
      auto FunctionID = iter["FunctionID"].GetUint64();
      auto LoopID = iter["LoopID"].GetUint64();
      auto didRec = iter["InstrumentedLoop"].GetBool();
      if (didRec) {
        results->didRecordFunctionDependencies(ModuleID, FunctionID, LoopID);
      }
      for (auto &dep : iter[DependenciesKey].GetArray()) {
        assert(didRec && "how did we get here??");
        auto depType = StringRef(dep[0].GetString());
        auto first = dep[1].GetUint64();
        auto firstValue = dyn_cast<Instruction>((*mapping)[first]);
        assert(firstValue);
        auto second = dep[2].GetUint64();
        auto secondValue = dyn_cast<Instruction>((*mapping)[second]);
        assert(secondValue);
        auto dependency = std::pair<const Value *, const Value *>(getPtrValue(firstValue), getPtrValue(secondValue));
        if (isRaR(depType)) {
          results->addFunctionRaR(ModuleID, FunctionID, LoopID, dependency);
        } else if (isRaW(depType)) {
          results->addFunctionRaW(ModuleID, FunctionID, LoopID, dependency);
        } else if (isWaR(depType)) {
          results->addFunctionWaR(ModuleID, FunctionID, LoopID, dependency);
        } else if (isWaW(depType)) {
          results->addFunctionWaW(ModuleID, FunctionID, LoopID, dependency);
        } else {
          errs() << "depType not known: " << depType << '\n';
          assert(0 && "found an unknown dependency type");
        }
        errs() << "parsing response " << depType << ":";
        getPtrValue(firstValue)->print(errs());
        errs() << " ";
        getPtrValue(secondValue)->print(errs());
        errs() << '\n';
      }
    }
  }

}

Value *VIAInvoker::getPtrValue(Instruction *I) {
  if (auto *Load = dyn_cast<LoadInst>(I)) {
    return Load->getPointerOperand();
  } else if (auto *Store = dyn_cast<StoreInst>(I)) {
    return Store->getPointerOperand();
    // FIXME: handle calls correctly.
  } else if ( auto *Call = dyn_cast<CallInst>(I) ) {
    return I;
  } else if ( auto AllocA = dyn_cast<AllocaInst>(I) ) {
    return I;
  } else {
      errs() << "Got an Instruction with opname: " <<  I->getOpcodeName() << '\n';
      assert ( 0 && "Instruction must be a load, store, alloca or a call" );
  }
}

std::shared_ptr<OracleAliasResults> VIAInvoker::getResults() {
  assert( results && "have not yet computed AA results" );
  return results;
}
