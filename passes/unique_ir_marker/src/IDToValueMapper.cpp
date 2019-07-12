#include <IDToValueMapper.hpp>

#include "UniqueIRMarkerReader.hpp"
#include "IDToValueMapper.hpp"

using std::addressof;

IDToInstructionMapper::IDToInstructionMapper(Module &M) : InstVisitor<IDToInstructionMapper>(), Mod(M), relevantIDs(nullptr) {}


std::unique_ptr<std::map<IDType, Instruction *>> IDToInstructionMapper::idToValueMap(std::set<IDType> &IDs) {
  relevantIDs = &IDs;
  auto map = std::make_unique<std::map<IDType, Instruction *>>();
  mapping = map.get();
  this->visit(Mod);
  return map;

}

void IDToInstructionMapper::visitInstruction(Instruction &I) {
  auto InstructionIDOpt = UniqueIRMarkerReader::getInstructionID(addressof(I));
  if (!InstructionIDOpt) return;
  auto IID = InstructionIDOpt.value();
  if ( relevantIDs->find(IID) != relevantIDs->end() ) {
    mapping->insert( std::pair<IDType, Instruction *>(IID, addressof(I)) );
  }
}

IDToFunctionMapper::IDToFunctionMapper(Module &M) : InstVisitor<IDToFunctionMapper>(),
    Mod(M), relevantIDs(nullptr) {}


std::unique_ptr<std::map<IDType, Function *>> IDToFunctionMapper::idToValueMap(std::set<IDType> &IDs) {
  relevantIDs = &IDs;
  auto map = std::make_unique<std::map<IDType, Function *>>();
  mapping = map.get();
  this->visit(Mod);
  return map;

}

void IDToFunctionMapper::visitFunction(Function &F) {
  auto FunctionIDOpt = UniqueIRMarkerReader::getFunctionID(addressof(F));
  if (!FunctionIDOpt) return;
  auto FID = FunctionIDOpt.value();
  if ( relevantIDs->find(FID) != relevantIDs->end() ) {
    mapping->insert( std::pair<IDType, Function *>(FID, &F) );
  }
}

