#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include <set>

#include "../include/PDG.hpp"

void llvm::PDG::computeGraphFor (Module &M, ModuleAliasInfo *aa){
  aaInfo = aa;

  collectAliasPairs(M);
  constructNodes(M);
  constructUseDefEdges(M);
  constructAliasingEdges(M);

  return ;
}

void llvm::PDG::collectAliasPairs (Module &M){
  // TODO:
  /*
   * Iterate over store and loads, collecting may/must alias information between each pair of them
   */
}

void llvm::PDG::constructNodes (Module &M){

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

void llvm::PDG::constructUseDefEdges (Module &M){
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

void llvm::PDG::constructAliasingEdges (Module &M){
  // TODO:
  /*
   * Use alias information on stores and loads to construct edges between pairs of these instructions
   */
}
