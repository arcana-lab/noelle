#ifndef ORACLEDDGAA_UNIQUEIRMARKER_H
#define ORACLEDDGAA_UNIQUEIRMARKER_H

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Pass.h>
#include <llvm/IR/InstVisitor.h>

using namespace llvm;

using IDType = uint64_t;

class UniqueIRMarker : public InstVisitor<UniqueIRMarker> {

 public:
  explicit UniqueIRMarker(ModulePass& MP);

  void visitModule    (Module &M);
  void visitFunction  (Function &F);
  void visitBasicBlock(BasicBlock &BB);
  void visitInstruction(Instruction &I);

  constexpr static unsigned int IDSize = sizeof(IDType) * 8; // IDs are in bits

 private:


  IDType uniqueInstructionCounter();

  IDType BasicBlockCounter, InstructionCounter, FunctionCounter, LoopCounter, ModuleCounter;
  MDNode* buildNode(LLVMContext&, IDType);
  ModulePass& MP;


  const StringRef VIAInstruction = "VIA.Ins.ID";
  const StringRef VIABasicBlock = "VIA.BB.ID";
  const StringRef VIAFunction = "VIA.Fn.ID";
  const StringRef VIAModule = "VIA.M.ID";

};

class UniqueIRMarkerPass : public ModulePass {
 public:
  static char ID;

  UniqueIRMarkerPass();
  bool doInitialization (Module &M) override ;
  void getAnalysisUsage(AnalysisUsage& ) const override;
  bool runOnModule(Module& ) override;
};

#endif //ORACLEDDGAA_UNIQUEIRMARKER_H
