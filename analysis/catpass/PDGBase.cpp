#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include "../include/PDGBase.hpp"

llvm::PDGEdge::PDGEdge(PDGNodeBase<Instruction> *src, PDGNodeBase<Instruction> *dst)
  : from(src), to(dst), memory(false), must(false), readAfterWrite(false), writeAfterWrite(false) {}

std::string llvm::PDGEdge::toString() {
  std::string edgeStr;
  raw_string_ostream ros(edgeStr);
  ros << (readAfterWrite ? "RAW " : (writeAfterWrite ? "WAW " : ""));
  ros << (must ? "(must) " : "(may) ");
  ros << (memory ? "from memory " : "") << "\n";
  return edgeStr;
}

bool llvm::PDGEdge::isMemoryDependence() {
  return memory;
}

void llvm::PDGEdge::setMemMustRaw(bool mem, bool must, bool raw) {
  this->memory = mem;
  this->must = must;
  this->readAfterWrite = raw;
  this->writeAfterWrite = !raw;
}
