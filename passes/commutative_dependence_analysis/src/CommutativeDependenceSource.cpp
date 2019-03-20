

#include <llvm/ADT/StringRef.h>
#include <llvm/Support/CommandLine.h>

#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/istreamwrapper.h>

#include <IDToValueMapper.hpp>
#include <CommutativeDependenceSource.hpp>

#include "CommutativeDependenceSource.hpp"

using namespace llvm;

cl::opt<std::string> CommutativeDependencyFilename("comm-dep", cl::desc("A list of all  the commutative"
                                                                      " dependencies in the current compilation "
                                                                      "unit."),
                                                             cl::value_desc("filename"), cl::ValueOptional);

void CommutativeDependenceSource::loadCommutativeDependencies() {
  if (!CommutativeDependencyFilename.empty()) {
    auto SR = StringRef(CommutativeDependencyFilename.c_str());
    loadFromFile(SR);
  }
}
void CommutativeDependenceSource::loadFromFile(StringRef &filename) {
  std::ifstream commDepFile(filename);

  if ( !commDepFile.good() ) {
    assert (0 && "Commutative Dependenecy file not good.");
  }

  rapidjson::Document doc;
  rapidjson::IStreamWrapper commDepFileWrapper(commDepFile);
  doc.ParseStream(commDepFileWrapper);

  if (!doc.IsObject()) assert(0 && "Empty file pass as '-comm-dep' argument");

  std::set<IDType> InstructionIDs;
  std::set<IDType> FunctionIDs;
  for ( auto &function : doc["Functions"].GetArray() ) {
    FunctionIDs.insert(function["FunctionID"].GetUint64());
    for ( auto &dependency : function["Dependencies"].GetArray() )  {
      auto left = dependency[0].GetUint64();
      auto right = dependency[1].GetUint64();
      InstructionIDs.insert(left);
      InstructionIDs.insert(right);
    }
  }

  IDToInstructionMapper InstructionValueMapper { M };
  auto InstructionMapping = InstructionValueMapper.idToValueMap(InstructionIDs);

  IDToFunctionMapper FunctionValueMapper { M };
  auto FunctionMapping = FunctionValueMapper.idToValueMap(FunctionIDs);

  for ( auto &function : doc["Functions"].GetArray() ) {
    auto F = (*FunctionMapping)[function["FunctionID"].GetUint64()];
    for ( auto &dependency : function["Dependencies"].GetArray() ) {
      auto left = dependency[0].GetUint64();
      auto right = dependency[1].GetUint64();
      auto leftValue = (*InstructionMapping)[left];
      auto rightValue = (*InstructionMapping)[right];
      CommutativeDependencies[F].insert(make_pair(leftValue, rightValue));
    }
  }

}

CommutativeDependenceSource::CommutativeDependenceSource(llvm::Module &M): M(M) {}



