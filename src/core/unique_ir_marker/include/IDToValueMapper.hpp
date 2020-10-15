#pragma once

#include "SystemHeaders.hpp"

#include "UniqueIRMarker.hpp"

namespace llvm::noelle {

  class IDToInstructionMapper  : public InstVisitor<IDToInstructionMapper> {
  public:
    explicit IDToInstructionMapper(Module &);

    std::unique_ptr<std::map<IDType, Instruction *>> idToValueMap(std::set<IDType> &);

    void visitInstruction(Instruction &I);

  private:
    std::set<IDType> *relevantIDs;
    std::map<IDType, Instruction *> *mapping;
    Module &Mod;
  };

  class IDToFunctionMapper : public InstVisitor<IDToFunctionMapper> {
  public:
    explicit IDToFunctionMapper(Module &);

    std::unique_ptr<std::map<IDType, Function *>> idToValueMap(std::set<IDType> &);

    void visitFunction(Function &I);

  private:
    std::set<IDType> *relevantIDs;
    std::map<IDType, Function *> *mapping;
    Module &Mod;
  };

}
