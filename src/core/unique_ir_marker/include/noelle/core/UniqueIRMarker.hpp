#pragma once

#include "noelle/core/SystemHeaders.hpp"

#include "noelle/core/UniqueIRConstants.hpp"
#include "noelle/core/UniqueIRMarkerReader.hpp"

using namespace llvm;

namespace llvm::noelle {

enum class MarkerMode { Instrument, Reinstrument, Renumber };

class UniqueIRMarker : public InstVisitor<UniqueIRMarker> {

public:
  UniqueIRMarker(ModulePass &MP, MarkerMode mode);

  void visitModule(Module &M);
  void visitFunction(Function &F);
  void visitBasicBlock(BasicBlock &BB);
  void visitInstruction(Instruction &I);

  constexpr static unsigned int IDSize = sizeof(IDType) * 8; // IDs are in bits

private:
  IDType uniqueInstructionCounter();
  IDType uniqueModuleCounter();

  IDType BasicBlockCounter, InstructionCounter, FunctionCounter, LoopCounter,
      ModuleCounter;
  MDNode *buildNode(LLVMContext &, IDType);
  ModulePass &MP;

  MarkerMode Mode;
};

} // namespace llvm::noelle
