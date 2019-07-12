#pragma once

#include "SystemHeaders.hpp"
#include <optional>

#include "UniqueIRConstants.hpp"

using std::optional;
using std::nullopt;
using std::reference_wrapper;

using namespace llvm;

class UniqueIRMarkerReader {

  friend class UniqueIRMarker;
  friend class UniqueIRVerifier;

 public:
  static optional<Constant *> getInstructionConstID(const llvm::Instruction *);
  static optional<Constant *> getFunctionConstID(const llvm::Function *);
  static optional<Constant *> getBasicBlockConstID(const llvm::BasicBlock *);
  static optional<Constant *> getLoopConstID(const llvm::Loop *);
  static optional<Constant *> getModuleConstID(const llvm::Module *);

  static optional<IDType> getModuleID(const llvm::Module *);
  static optional<IDType> getFunctionID(const llvm::Function *);
  static optional<IDType> getBasicBlockID(const llvm::BasicBlock *);
  static optional<IDType> getInstructionID(const llvm::Instruction *);
  static optional<IDType> getLoopID(const llvm::Loop *);

 private:
  static optional<IDType> getID(const Constant *);
  static optional<Constant *> getConst(const llvm::MDOperand &);
  static optional<reference_wrapper<const MDOperand>> getIthOperand(const MDNode *, uint operand);

  template <typename T>
  static optional<IDType> getIDConst(const T *, std::function<optional<Constant *>(const T *)>);

  static optional<Constant *> getConstFromMeta(llvm::MDNode *, uint operand);

  static optional <IDType> getIDFromLoopMeta(MDNode *);
};
