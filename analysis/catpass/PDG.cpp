#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
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
  if (F.empty()) return nullptr;
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
   * Recreate all edges connected only within nodes of function F
   */
  for (auto *oldEdge : allEdges) {
    /*
     * Copy edge to new PDG, replacing connected nodes
     */
    if (oldEdge->belongsTo(F)) {
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

PDG *llvm::PDG::createLoopsSubgraph(LoopInfo &LI) {
  if (LI.empty()) return nullptr;
  auto loopsPDG = new PDG();

  /*
   * Create a node per instruction within loops of LI only
   */
  for (auto i : LI) {
    Loop *loop = &*i;
    for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi){
      BasicBlock *B = *bbi;
      for (auto &I : *B) {
        loopsPDG->createNodeFrom(&I);
      }
    }
  }

  /*
   * Set the entry node: the first instruction of one of the top level loops (See include/llvm/Analysis/LoopInfo.h:653)
   */
  Loop *loopBegin = *(LI.begin());
  BasicBlock *bbBegin = *(loopBegin->block_begin());
  loopsPDG->entryNode = loopsPDG->instructionNodes[&*(bbBegin->begin())];
  assert(loopsPDG->entryNode != nullptr);

  /*
   * Recreate all edges connected only within nodes of loops of LI
   */
  for (auto *oldEdge : allEdges) {
    auto nodePair = oldEdge->getNodePair();
    auto fromPair = loopsPDG->instructionNodes.find(nodePair.first->getNode());
    auto toPair = loopsPDG->instructionNodes.find(nodePair.second->getNode());

    /*
     * Copy edge to new PDG, replacing connected nodes
     */
    if (fromPair != loopsPDG->instructionNodes.end() && toPair != loopsPDG->instructionNodes.end()) {
      auto *edge = new PDGEdge(*oldEdge);
      auto fromNode = fromPair->second;
      auto toNode = toPair->second;
      edge->setNodePair(fromNode, toNode);

      loopsPDG->allEdges.push_back(edge);
      fromNode->addOutgoingNode(toNode, edge);
      toNode->addIncomingNode(fromNode, edge);
    }
   }

   return loopsPDG;
}