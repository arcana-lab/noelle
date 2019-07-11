
#include <UniqueIRMarkerReader.hpp>

#include "UniqueIRConstants.hpp"
#include "UniqueIRMarkerReader.hpp"


using namespace llvm;



optional<Constant *> UniqueIRMarkerReader::getInstructionConstID(const llvm::Instruction *I) {
  if (!I) return nullopt;
  return getConstFromMeta(I->getMetadata(UniqueIRConstants::VIAInstruction), 0);
}

optional<Constant *> UniqueIRMarkerReader::getModuleConstID(const llvm::Module *M) {
  assert(M && "Not a valid Module");
  auto* metaNode = M->getNamedMetadata(Twine(UniqueIRConstants::VIAModule));
  assert(metaNode && "No ID for module it is likely that the unique IR identifier pass (adding metadata with IDs) has "
                     "not been run");
  assert(metaNode->getNumOperands() == 1 && "Should only have one meta node operand");
  assert(metaNode->getNumOperands() == 1 && metaNode->getOperand(0)->getNumOperands() == 1 && "Not in correct format");
  return getConst(metaNode->getOperand(0)->getOperand(0));
}

optional<Constant *> UniqueIRMarkerReader::getLoopConstID(const llvm::Loop *L) {
  assert(L && "Not a valid Loop");
  auto* metaNode = L->getLoopID();
  if (!metaNode || metaNode->getNumOperands() < 2) {
    return nullopt;
  }

  auto *possibleVIAIDPair = dyn_cast<MDTuple>(metaNode->getOperand(1));
  if(!possibleVIAIDPair || possibleVIAIDPair->getNumOperands() < 2) {
    return nullopt;
  }

  auto possibleVIAStrID = dyn_cast<MDString>(possibleVIAIDPair->getOperand(0));
  if (!possibleVIAStrID || !possibleVIAStrID->getString().equals(UniqueIRConstants::VIALoop)) {
    return nullopt;
  }

  auto possibleVIAID = dyn_cast<ConstantAsMetadata>(possibleVIAIDPair->getOperand(1));
  if (!possibleVIAID) {
    return nullopt;
  }
  return optional<Constant *>(possibleVIAID->getValue());
}

optional<Constant *> UniqueIRMarkerReader::getBasicBlockConstID(const llvm::BasicBlock *BB) {
  assert(!BB->empty() && "Empty Basic Blocks have no ID");
  auto* metaNode = BB->front().getMetadata(UniqueIRConstants::VIABasicBlock);
  return getConstFromMeta(metaNode, 0);
}

optional<Constant *> UniqueIRMarkerReader::getFunctionConstID(const llvm::Function *F) {
  assert(F && "Not a valid function");
  auto* metaNode = F->getMetadata(UniqueIRConstants::VIAFunction);
  return getConstFromMeta(metaNode, 0);
}

optional<IDType> UniqueIRMarkerReader::getModuleID(const Module *M) {
  return getIDConst<Module>(M, getModuleConstID);
}

optional<IDType> UniqueIRMarkerReader::getFunctionID(const llvm::Function* F) {
  return getIDConst<Function>(F, getFunctionConstID);
}

optional<IDType> UniqueIRMarkerReader::getBasicBlockID(const llvm::BasicBlock* BB) {
  return getIDConst<BasicBlock>(BB, getBasicBlockConstID);
}

optional<IDType> UniqueIRMarkerReader::getInstructionID(const llvm::Instruction* I) {
  return getIDConst<Instruction>(I, getInstructionConstID);
}

optional<IDType> UniqueIRMarkerReader::getLoopID(const llvm::Loop* L) {
  return getIDConst<Loop>(L, getLoopConstID);
}

optional<IDType> UniqueIRMarkerReader::getID(const Constant *C) {
  auto ConstInt = dyn_cast<ConstantInt>(C);
  return  ConstInt ? optional<IDType>(ConstInt->getZExtValue()) : nullopt;
}

optional<Constant *> UniqueIRMarkerReader::getConst(const llvm::MDOperand& node) {
  auto *ConstMeta = dyn_cast<ConstantAsMetadata>(node);
  return ConstMeta ? optional<Constant *>(ConstMeta->getValue()) : nullopt;
}

optional<reference_wrapper<const MDOperand>> UniqueIRMarkerReader::getIthOperand(const MDNode* node, uint operand) {
  if (node && node->getNumOperands() == (operand + 1)) {
    return optional<reference_wrapper<const MDOperand>>{ node->getOperand(operand) };
  }
  return std::nullopt;
}

template< typename T>
optional<IDType> UniqueIRMarkerReader::getIDConst(const T *t, std::function<optional<Constant *>(const T *)> f) {
  auto *x = f(t).value_or(nullptr);
  return x ? getID(x) : nullopt;
}

optional<Constant *> UniqueIRMarkerReader::getConstFromMeta(llvm::MDNode *node, uint operand) {
  if (!node) return nullopt;
  auto np = getIthOperand(node, operand);
  return np ? getConst(np.value()) : nullopt;
}

optional<IDType> UniqueIRMarkerReader::getIDFromLoopMeta(MDNode *Node) {
  assert(0 && "fix this");
  auto Const = getConstFromMeta(Node, 0);
  if (Const) {
    return getID(Const.value());
  }
  return nullopt;
}


