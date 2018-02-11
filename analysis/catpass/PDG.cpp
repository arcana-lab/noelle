#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include <set>

#include "../include/PDG.hpp"

llvm::PDG::PDG() {}

llvm::PDG::~PDG() {
  errs() << "Destroying PDG\n";
  for (auto *edge : allEdges)
    if (edge) delete edge;
  for (auto *node : allNodes)
    if (node) delete node;
}

void llvm::PDG::constructNodes (Module &M) {
  /*
   * Create a node per instruction.
   */
  for (auto &F : M) {
    for (auto &B : F) {
      for (auto &I : B) {
        PDGNodeBase<Instruction> *node = new PDGNodeBase<Instruction>(&I);
        allNodes.push_back(node);
        instructionNodes[&I] = node;
      }
    }
  }

  /* 
   * Set the entry node: the first instruction of the function "main"
   */
  auto mainF = M.getFunction("main");
  if (mainF == nullptr) {
    errs() << "ERROR: Main function not found\n";
    abort();
  }
  auto entryInstr = &*(mainF->begin()->begin());
  entryNode = instructionNodes[entryInstr];
  assert(entryNode != nullptr);

  return ;
}

PDGEdge *llvm::PDG::addEdgeFromTo(Instruction *from, Instruction *to) {
  auto fromNode = instructionNodes[from];
  auto toNode = instructionNodes[to];
  auto edge = new PDGEdge(fromNode, toNode);
  allEdges.push_back(edge);
  fromNode->addOutgoingNode(toNode, edge);
  toNode->addIncomingNode(fromNode, edge);
  return edge;
}