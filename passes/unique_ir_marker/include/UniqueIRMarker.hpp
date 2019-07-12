#pragma once

#include "SystemHeaders.hpp"

#include "UniqueIRConstants.hpp"
#include "UniqueIRMarkerReader.hpp"

using namespace llvm;


enum class MarkerMode {
  Instrument,
  Reinstrument,
  Renumber
};

class UniqueIRMarker : public InstVisitor<UniqueIRMarker> {

 public:
  UniqueIRMarker(ModulePass& MP, MarkerMode mode);

  void visitModule    (Module &M);
  void visitFunction  (Function &F);
  void visitBasicBlock(BasicBlock &BB);
  void visitInstruction(Instruction &I);

  constexpr static unsigned int IDSize = sizeof(IDType) * 8; // IDs are in bits

 private:


  IDType uniqueInstructionCounter();
  IDType uniqueModuleCounter();

  IDType BasicBlockCounter, InstructionCounter, FunctionCounter, LoopCounter, ModuleCounter;
  MDNode* buildNode(LLVMContext&, IDType);
  ModulePass& MP;

  MarkerMode Mode;

};


