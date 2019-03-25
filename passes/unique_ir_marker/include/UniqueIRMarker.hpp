#pragma once

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Pass.h>
#include <llvm/IR/InstVisitor.h>

#include <set>

#include "UniqueIRConstants.hpp"
#include "UniqueIRMarkerReader.hpp"

using namespace llvm;


class UniqueIRMarker : public InstVisitor<UniqueIRMarker> {

 public:
  explicit UniqueIRMarker(ModulePass& MP);

  void visitModule    (Module &M);
  void visitFunction  (Function &F);
  void visitBasicBlock(BasicBlock &BB);
  void visitInstruction(Instruction &I);

  constexpr static unsigned int IDSize = sizeof(IDType) * 8; // IDs are in bits

  bool verifyLoops();

 private:


  IDType uniqueInstructionCounter();
  IDType uniqueModuleCounter();

  IDType BasicBlockCounter, InstructionCounter, FunctionCounter, LoopCounter, ModuleCounter;
  MDNode* buildNode(LLVMContext&, IDType);
  ModulePass& MP;

  // if the bitcode file already has a module 'VIA.M.ID' definition then this walker will instead verify that
  // the metadata is correctly inserted.
  bool AlreadyMarked = false;

  std::set<IDType> LoopIDsFromLoopInfo;
  std::set<IDType> LoopIDsFromPreviousMarkerPass;

  void checkFunction(Function &);
  void checkInstruction(Instruction &);



};

class UniqueIRMarkerPass : public ModulePass {
 public:
  static char ID;

  UniqueIRMarkerPass();
  bool doInitialization (Module &M) override ;
  void getAnalysisUsage(AnalysisUsage& ) const override;
  bool runOnModule(Module& ) override;
};
