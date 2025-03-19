#ifndef NOELLE_SRC_CORE_UNIQUE_IR_MARKER_UNIQUEIRMARKER_H_
#define NOELLE_SRC_CORE_UNIQUE_IR_MARKER_UNIQUEIRMARKER_H_

#include "arcana/noelle/core/SystemHeaders.hpp"
#include "arcana/noelle/core/UniqueIRConstants.hpp"
#include "arcana/noelle/core/UniqueIRMarkerReader.hpp"

using namespace llvm;

namespace arcana::noelle {

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
  MDNode *buildNode(LLVMContext &, IDType);

  ModulePass &MP;
  MarkerMode Mode;
  IDType BasicBlockCounter, FunctionCounter, InstructionCounter, LoopCounter,
      ModuleCounter;
};

} // namespace arcana::noelle

#endif // NOELLE_SRC_CORE_UNIQUE_IR_MARKER_UNIQUEIRMARKER_H_
