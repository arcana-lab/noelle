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
  //errs() << "Destroying PDG\n";
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
  entryNode = nodeMap[entryInstr];
  assert(entryNode != nullptr);

  return ;
}

DGNode<Instruction> *llvm::PDG::createNodeFrom(Instruction *I) {
  auto *node = new DGNode<Instruction>(I);
  allNodes.push_back(node);
  nodeMap[I] = node;
}

DGEdge<Instruction> *llvm::PDG::createEdgeFromTo(Instruction *from, Instruction *to) {
  auto fromNode = nodeMap[from];
  auto toNode = nodeMap[to];
  auto edge = new DGEdge<Instruction>(fromNode, toNode);
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
  functionPDG->entryNode = functionPDG->nodeMap[&*(F.begin()->begin())];
  assert(functionPDG->entryNode != nullptr);

  /*
   * Recreate all edges connected only within nodes of function F
   */
  for (auto *oldEdge : allEdges) {
    /*
     * Copy edge to new PDG, replacing connected nodes
     */
    if (oldEdge->belongsTo(F)) {
      auto *edge = new DGEdge<Instruction>(*oldEdge);
      auto edgeNodePair = oldEdge->getNodePair();
      auto fromNode = functionPDG->nodeMap[edgeNodePair.first->getNode()];
      auto toNode = functionPDG->nodeMap[edgeNodePair.second->getNode()];
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
  loopsPDG->entryNode = loopsPDG->nodeMap[&*(bbBegin->begin())];
  assert(loopsPDG->entryNode != nullptr);

  /*
   * Recreate all edges connected only within nodes of loops of LI
   */
  for (auto *oldEdge : allEdges) {
    auto nodePair = oldEdge->getNodePair();
    auto fromPair = loopsPDG->nodeMap.find(nodePair.first->getNode());
    auto toPair = loopsPDG->nodeMap.find(nodePair.second->getNode());

    /*
     * Copy edge to new PDG, replacing connected nodes
     */
    if (fromPair != loopsPDG->nodeMap.end() && toPair != loopsPDG->nodeMap.end()) {
      auto *edge = new DGEdge<Instruction>(*oldEdge);
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