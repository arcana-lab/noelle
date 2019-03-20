#ifndef COMMUTATIVITY_ANALYSIS_HPP
#define COMMUTATIVITY_ANALYSIS_HPP

#include <llvm/Analysis/LoopInfo.h>
#include <llvm/Pass.h>
#include <llvm/IR/InstVisitor.h>

#include "CommutativeDependenceSource.hpp"

using namespace llvm;

//class UniqueIRMarker : public InstVisitor<UniqueIRMarker> {
//
// public:
//  explicit UniqueIRMarker(ModulePass& MP);
//
//  void visitModule    (Module &M);
//  void visitFunction  (Function &F);
//  void visitBasicBlock(BasicBlock &BB);
//  void visitInstruction(Instruction &I);

//  constexpr static unsigned int IDSize = sizeof(IDType) * 8; // IDs are in bits

// private:

//
//  IDType uniqueInstructionCounter();
//
//  IDType BasicBlockCounter, InstructionCounter, FunctionCounter, LoopCounter, ModuleCounter;
//  MDNode* buildNode(LLVMContext&, IDType);
//  ModulePass& MP;

//};

class CommutativeDependenceAnalysisPass : public ModulePass {
 public:
  static char ID;

  CommutativeDependenceAnalysisPass();
  bool doInitialization (Module &M) override ;
  void getAnalysisUsage(AnalysisUsage& ) const override;
  bool runOnModule(Module& ) override;

  iterator_range<CommutativeDependenceSource::comm_dep_iterator>
  getRemovedDependencies(Function *);

 private:

  unique_ptr<CommutativeDependenceSource> CommDepSource;
};

#endif
