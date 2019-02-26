
#include <UniqueIRMarkerReader.hpp>

#include "UniqueIRMarkerReader.hpp"

using namespace llvm;


const StringRef UniqueIRMarkerReader::VIAInstruction = "VIA.Ins.ID";
const StringRef UniqueIRMarkerReader::VIABasicBlock = "VIA.BB.ID";
const StringRef UniqueIRMarkerReader::VIAFunction = "VIA.Fn.ID";
const StringRef UniqueIRMarkerReader::VIAModule = "VIA.M.ID";

llvm::Constant *UniqueIRMarkerReader::getInstructionConstID(const llvm::Instruction *I) {
  assert(I && "Not a valid Instruction");
  auto* metaNode = I->getMetadata(VIAInstruction);
  return getConst(getIthOperand(metaNode, 0));
}

llvm::Constant *UniqueIRMarkerReader::getModuleConstID(const llvm::Module *M) {
  assert(M && "Not a valid Module");
  auto* metaNode = M->getNamedMetadata(Twine(VIAModule));
  assert(metaNode && "No ID for module it is likely that the unique IR identifier pass (adding metadata with IDs) has "
                     "not been run");
  assert(metaNode->getNumOperands() == 1 && "Should only have one meta node operand");
  assert(metaNode->getNumOperands() == 1 && metaNode->getOperand(0)->getNumOperands() == 1 && "Not in correct format");
  return getConst(metaNode->getOperand(0)->getOperand(0));
}

Constant *UniqueIRMarkerReader::getLoopConstID(const llvm::Loop *L) {
  assert(L && "Not a valid Loop");
  auto* metaNode = L->getLoopID();
  return getConst(getIthOperand(metaNode, 1));
}

Constant *UniqueIRMarkerReader::getBasicBlockConstID(const llvm::BasicBlock *BB) {
  assert(!BB->empty() && "Empty Basic Blocks have no ID");
  auto* metaNode = BB->front().getMetadata(VIABasicBlock);
  return getConst(getIthOperand(metaNode, 0));
}

Constant *UniqueIRMarkerReader::getFunctionConstID(const llvm::Function *F) {
  assert(F && "Not a valid function");
  auto* metaNode = F->getMetadata(VIAFunction);
  if (!metaNode) return nullptr;
  return getConst(getIthOperand(metaNode, 0));
}

IDType UniqueIRMarkerReader::getModuleID(const Module *M) {
  return getID(getModuleConstID(M));
}

IDType UniqueIRMarkerReader::getFunctionID(const llvm::Function* F) {
  return getID(getFunctionConstID(const_cast<Function *>(F)));
}

IDType UniqueIRMarkerReader::getBasicBlockID(const llvm::BasicBlock* BB) {
  return getID(getBasicBlockConstID(BB));
}

IDType UniqueIRMarkerReader::getInstructionID(const llvm::Instruction* I) {
  return getID(getInstructionConstID(I));
}

IDType UniqueIRMarkerReader::getLoopID(const llvm::Loop* L) {
  return getID(getLoopConstID(L));
}

IDType UniqueIRMarkerReader::getID(const Constant *C) {
  return dyn_cast<ConstantInt>(C)->getZExtValue();
}

Constant *UniqueIRMarkerReader::getConst(const llvm::MDOperand& node) {
  return dyn_cast<ConstantAsMetadata>(node)->getValue();
}

const MDOperand& UniqueIRMarkerReader::getIthOperand(const MDNode* node, uint operand) {
  assert(node && "Metadata not found");
  assert(node->getNumOperands() == (operand+1) && "Meta node doesn't have the correct number of operands");

  return node->getOperand(operand);

}

