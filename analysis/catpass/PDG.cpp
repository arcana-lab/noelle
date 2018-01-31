#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "../include/PDG.hpp"

llvm::PDG::PDG (Module &M){
  constructNodes(M);
  constructEdges(M);

  return ;
}

void llvm::PDG::constructNodes (Module &M){
  for (auto &F : M) {
    for (auto &B : F) {
      for (auto &I : B) {
        PDGNodeBase<Instruction> *node = new PDGNodeBase<Instruction>(&I);
        allNodes.push_back(node);
        instructionNodes[&I] = node;
      }
    }
  }
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

void llvm::PDG::constructEdges (Module &M){
  for (auto &F : M) {
    for (auto &B : F) {
      for (auto &I : B) {
        PDGNodeBase<Instruction> *iNode = instructionNodes[&I];
        if (I.getNumUses() == 0)
          continue;
        for (auto& U : I.uses()) {
          auto user = U.getUser();
          if (auto userInst = dyn_cast<Instruction>(user)){
            auto targetNode = instructionNodes[userInst];
            auto edge = new PDGEdge(iNode, targetNode);
            allEdges.push_back(edge);
            iNode->addOutgoingNode(targetNode, edge);
            targetNode->addIncomingNode(iNode, edge);
          }
        }
      }
    }
  }

  return ;
}
