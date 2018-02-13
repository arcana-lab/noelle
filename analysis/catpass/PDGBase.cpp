#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include "../include/PDGBase.hpp"

llvm::PDGEdge::PDGEdge(PDGNodeBase<Instruction> *src, PDGNodeBase<Instruction> *dst)
  : from(src), to(dst), memory(false), must(false), readAfterWrite(false), writeAfterWrite(false) {}

llvm::PDGEdge::PDGEdge(const PDGEdge &oldEdge) {
  auto nodePair = oldEdge.getNodePair();
  from = nodePair.first;
  to = nodePair.second;
  setMemMustRaw(oldEdge.isMemoryDependence(), oldEdge.isMustDependence(), oldEdge.isRAWDependence());
}

std::string llvm::PDGEdge::toString() {
  std::string edgeStr;
  raw_string_ostream ros(edgeStr);
  ros << (readAfterWrite ? "RAW " : (writeAfterWrite ? "WAW " : ""));
  ros << (must ? "(must) " : "(may) ");
  ros << (memory ? "from memory " : "") << "\n";
  return edgeStr;
}

void llvm::PDGEdge::setMemMustRaw(bool mem, bool must, bool raw) {
  this->memory = mem;
  this->must = must;
  this->readAfterWrite = raw;
  this->writeAfterWrite = !raw;
}

bool llvm::PDGEdge::belongsTo(Function &F) {
  return from->getNode()->getFunction() == &F && to->getNode()->getFunction() == &F;
}