
#include <IDToValueMapper.hpp>

#include "UniqueIRMarkerReader.hpp"
#include "IDToValueMapper.hpp"

IDToInstructionMapper::IDToInstructionMapper(Module &M) : InstVisitor<IDToInstructionMapper>(), Mod(M), relevantIDs(nullptr) {}


std::unique_ptr<std::map<IDType, Instruction *>> IDToInstructionMapper::idToValueMap(std::set<IDType> &IDs) {
  relevantIDs = &IDs;
  auto map = std::make_unique<std::map<IDType, Instruction *>>();
  mapping = map.get();
  this->visit(Mod);
  return map;

}

void IDToInstructionMapper::visitInstruction(Instruction &I) {
  // FIXME: handle not ID.
  auto ID = UniqueIRMarkerReader::getInstructionID(&I).value();
  if ( relevantIDs->find(ID) != relevantIDs->end() ) {
    mapping->insert( std::pair<IDType, Instruction *>(ID, &I) );
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
  // FIXME: handle not ID.
  auto ID = UniqueIRMarkerReader::getFunctionID(&F).value();
  if ( relevantIDs->find(ID) != relevantIDs->end() ) {
    mapping->insert( std::pair<IDType, Function *>(ID, &F) );
  }
}

