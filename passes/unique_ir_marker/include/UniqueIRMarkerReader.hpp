

#ifndef CAT_UNIQUEIRMARKERREADER_HPP
#define CAT_UNIQUEIRMARKERREADER_HPP

#include <llvm/IR/Constants.h>

#include "UniqueIRMarker.hpp"

class UniqueIRMarkerReader : public InstVisitor<UniqueIRMarker>  {

 public:
  static Constant* getInstructionConstID(const llvm::Instruction *);
  static Constant* getFunctionConstID(const llvm::Function *);
  static Constant* getBasicBlockConstID(const llvm::BasicBlock *);
  static Constant* getLoopConstID(const llvm::Loop *);
  static Constant* getModuleConstID(const llvm::Module *);

  static IDType getModuleID(const llvm::Module*);
  static IDType getFunctionID(const llvm::Function*);
  static IDType getBasicBlockID(const llvm::BasicBlock*);
  static IDType getInstructionID(const llvm::Instruction*);
  static IDType getLoopID(const llvm::Loop*);

  void visitInstruction(Instruction &I);



 private:
  static const StringRef VIAInstruction;
  static const StringRef VIABasicBlock;
  static const StringRef VIAFunction;
  static const StringRef VIAModule;

  static IDType getID(const Constant*);
  static Constant* getConst(const llvm::MDOperand&);
  static const MDOperand& getIthOperand(const MDNode*, uint operand);
};

#endif //CAT_UNIQUEIRMARKERREADER_HPP
