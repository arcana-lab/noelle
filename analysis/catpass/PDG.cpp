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
        createNodeFrom(&I);
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

PDGNodeBase<Instruction> *llvm::PDG::createNodeFrom(Instruction *I) {
  auto *node = new PDGNodeBase<Instruction>(I);
  allNodes.push_back(node);
  instructionNodes[I] = node;
}

PDGEdge *llvm::PDG::createEdgeFromTo(Instruction *from, Instruction *to) {
  auto fromNode = instructionNodes[from];
  auto toNode = instructionNodes[to];
  auto edge = new PDGEdge(fromNode, toNode);
  allEdges.push_back(edge);
  fromNode->addOutgoingNode(toNode, edge);
  toNode->addIncomingNode(fromNode, edge);
  return edge;
}

PDG *llvm::PDG::createFunctionSubgraph(Function &F) {
  auto functionPDG = new PDG();

  /*
   * Create a node per instruction of function F only
   */
  for (auto &B : F) {
    for (auto &I : B) {
      functionPDG->createNodeFrom(&I);
    }
  }

  /* 
   * Set the entry node: the first instruction of function F
   */

  functionPDG->entryNode = functionPDG->instructionNodes[&*(F.begin()->begin())];
  assert(functionPDG->entryNode != nullptr);

  /*
   * Recreate all edges connected only to nodes of function F
   */
   for (auto *oldEdge : allEdges) {
    if (oldEdge->belongsTo(F)) {
      /*
       * Copy edge to new PDG, replacing connected nodes
       */
      auto *edge = new PDGEdge(*oldEdge);
      auto edgeNodePair = oldEdge->getNodePair();
      auto fromNode = functionPDG->instructionNodes[edgeNodePair.first->getNode()];
      auto toNode = functionPDG->instructionNodes[edgeNodePair.second->getNode()];
      edge->setNodePair(fromNode, toNode);

      functionPDG->allEdges.push_back(edge);
      fromNode->addOutgoingNode(toNode, edge);
      toNode->addIncomingNode(fromNode, edge);
    }
   }

   return functionPDG;
}