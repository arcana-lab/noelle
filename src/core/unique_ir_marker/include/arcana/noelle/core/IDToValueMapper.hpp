#ifndef NOELLE_SRC_CORE_UNIQUE_IR_MARKER_IDTOVALUEMAPPER_H_
#define NOELLE_SRC_CORE_UNIQUE_IR_MARKER_IDTOVALUEMAPPER_H_

#include "arcana/noelle/core/SystemHeaders.hpp"

#include "arcana/noelle/core/UniqueIRMarker.hpp"

namespace arcana::noelle {

class IDToInstructionMapper : public InstVisitor<IDToInstructionMapper> {
public:
  explicit IDToInstructionMapper(Module &);

  std::unique_ptr<std::map<IDType, Instruction *>> idToValueMap(
      std::set<IDType> &);

  void visitInstruction(Instruction &I);

private:
  Module &Mod;
  std::set<IDType> *relevantIDs;
  std::map<IDType, Instruction *> *mapping;
};

class IDToFunctionMapper : public InstVisitor<IDToFunctionMapper> {
public:
  explicit IDToFunctionMapper(Module &);

  std::unique_ptr<std::map<IDType, Function *>> idToValueMap(
      std::set<IDType> &);

  void visitFunction(Function &I);

private:
  Module &Mod;
  std::set<IDType> *relevantIDs;
  std::map<IDType, Function *> *mapping;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_UNIQUE_IR_MARKER_IDTOVALUEMAPPER_H_
