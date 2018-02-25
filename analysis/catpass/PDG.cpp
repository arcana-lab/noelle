#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/LoopInfo.h"
#include <set>

#include "llvm/Support/raw_ostream.h"

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
        createNodeFrom(&I, /*inclusion=*/ true);
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
  entryNode = internalNodeMap[entryInstr];
  assert(entryNode != nullptr);

  return ;
}

PDG *llvm::PDG::createFunctionSubgraph(Function &F) {
  if (F.empty()) return nullptr;
  auto functionPDG = new PDG();

  /*
   * Create a node per instruction of function F only
   */
  for (auto &B : F) {
    for (auto &I : B) {
      functionPDG->createNodeFrom(&I, /*inclusion=*/ true);
    }
  }

  /* 
   * Set the entry node: the first instruction of function F
   */
  functionPDG->entryNode = functionPDG->internalNodeMap[&*(F.begin()->begin())];
  assert(functionPDG->entryNode != nullptr);

  /*
   * Recreate all edges connected only within nodes of function F
   */
  for (auto *oldEdge : allEdges) {
    auto edgeNodePair = oldEdge->getNodePair();
    auto fromInst = edgeNodePair.first->getNode();
    auto toInst = edgeNodePair.second->getNode();

    /*
     * Check whether edge belongs to nodes within function F
     */
    bool fromInclusion = fromInst->getFunction() == &F;
    bool toInclusion = toInst->getFunction() == &F;
    if (!fromInclusion && !toInclusion) continue;

    /*
     * Create appropriate external nodes and associate edge to them
     */
    auto fromNode = functionPDG->fetchOrCreateNodeOf(fromInst, fromInclusion);
    auto toNode = functionPDG->fetchOrCreateNodeOf(toInst, toInclusion);

    /*
     * Use edge copy constructor to match old edge properties (mem/var, must/may, RAW/WAW)
     */
    auto *edge = new DGEdge<Instruction>(*oldEdge);

    /*
     * Configure new edge/nodes to point to each other 
     */
    edge->setNodePair(fromNode, toNode);
    functionPDG->allEdges.push_back(edge);
    connectNodesVia(edge, fromNode, toNode);
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
        loopsPDG->createNodeFrom(&I, /*inclusion=*/ true);
      }
    }
  }

  /*
   * Set the entry node: the first instruction of one of the top level loops (See include/llvm/Analysis/LoopInfo.h:653)
   */
  Loop *loopBegin = *(LI.begin());
  BasicBlock *bbBegin = *(loopBegin->block_begin());
  loopsPDG->entryNode = loopsPDG->internalNodeMap[&*(bbBegin->begin())];
  assert(loopsPDG->entryNode != nullptr);

  /*
   * Recreate all edges connected only within nodes of loops of LI
   */
  for (auto *oldEdge : allEdges) {
    auto nodePair = oldEdge->getNodePair();
    auto fromNode = nodePair.first;
    auto toNode = nodePair.second;
    
    /*
     * Check whether edge belongs to nodes within loops of LI
     */
    bool fromInclusion = loopsPDG->isInternalNode(fromNode);
    bool toInclusion = loopsPDG->isExternalNode(toNode);
    if (!fromInclusion && !toInclusion) continue;

    /*
     * Create appropriate external nodes and associate edge to them
     */
    fromNode = loopsPDG->fetchOrCreateNodeOf(fromNode->getNode(), fromInclusion);
    toNode = loopsPDG->fetchOrCreateNodeOf(toNode->getNode(), toInclusion);

    /*
     * Use edge copy constructor to match old edge properties (mem/var, must/may, RAW/WAW)
     */
    auto *edge = new DGEdge<Instruction>(*oldEdge);

    /*
     * Configure new edge/nodes to point to each other 
     */
    edge->setNodePair(fromNode, toNode);
    loopsPDG->allEdges.push_back(edge);
    connectNodesVia(edge, fromNode, toNode);
   }

   return loopsPDG;
}