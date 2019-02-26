

#ifndef CAT_IDTOVALUEMAPPER_HPP
#define CAT_IDTOVALUEMAPPER_HPP

#include <set>
#include "UniqueIRMarker.hpp"

class IDToValueMapper  : public InstVisitor<IDToValueMapper> {
 public:
  IDToValueMapper(Module &);

  std::unique_ptr<std::map<IDType, Value*>> idToValueMap(std::set<IDType> &);

  void visitInstruction(Instruction &I);

 private:
  std::set<IDType> *relevantIDs;
  std::map<IDType, Value *> *mapping;
  Module &M;

};

#endif //CAT_IDTOVALUEMAPPER_HPP
