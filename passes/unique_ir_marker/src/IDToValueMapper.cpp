
#include "UniqueIRMarkerReader.hpp"
#include "IDToValueMapper.hpp"

IDToValueMapper::IDToValueMapper(Module &M) : InstVisitor<IDToValueMapper>(), M(M), relevantIDs(nullptr) {}


std::unique_ptr<std::map<IDType, Value *>> IDToValueMapper::idToValueMap(std::set<IDType> &IDs) {
  relevantIDs = &IDs;
  auto map = std::make_unique<std::map<IDType, Value *>>();
  mapping = map.get();
  this->visit(M);
  return map;

}

void IDToValueMapper::visitInstruction(Instruction &I) {
  auto ID = UniqueIRMarkerReader::getInstructionID(&I);
  if ( relevantIDs->find(ID) != relevantIDs->end() ) {
    mapping->insert( std::pair<IDType, Value *>(ID, &I) );
  }
}
