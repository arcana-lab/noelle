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
   * Recreate all edges connected to internal nodes of function
   */
  copyEdgesInto(functionPDG);

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
    for (auto bbi = loop->block_begin(); bbi != loop->block_end(); ++bbi) {
      for (auto &I : **bbi) {
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
   * Recreate all edges connected to internal nodes of loop
   */
  copyEdgesInto(loopsPDG);

  return loopsPDG;
}

PDG *llvm::PDG::createInstListSubgraph(std::vector<Instruction *> &instList) {
  if (instList.empty()) return nullptr;
  auto instPDG = new PDG();

  for (auto &I : instList) {
    instPDG->createNodeFrom(I, /*inclusion=*/ true);
  }

  instPDG->entryNode = instPDG->internalNodeMap[*(instList.begin())];
  assert(instPDG->entryNode != nullptr);

  copyEdgesInto(instPDG, /*linkToExternal=*/ false);

  return instPDG;
}

void llvm::PDG::copyEdgesInto(PDG *newPDG, bool linkToExternal) {
  for (auto *oldEdge : allEdges) {
    auto nodePair = oldEdge->getNodePair();
    auto fromT = nodePair.first->getT();
    auto toT = nodePair.second->getT();

    /*
     * Check whether edge belongs to nodes within function F
     */
    bool fromInclusion = newPDG->isInternal(fromT);
    bool toInclusion = newPDG->isInternal(toT);
    if (!fromInclusion && !toInclusion) continue;
    if (!linkToExternal && (!fromInclusion || !toInclusion)) continue;
    
    /*
     * Create appropriate external nodes and associate edge to them
     */
    auto newFromNode = newPDG->fetchOrCreateNodeOf(fromT, fromInclusion);
    auto newToNode = newPDG->fetchOrCreateNodeOf(toT, toInclusion);

    /*
     * Use edge copy constructor to match old edge properties (mem/var, must/may, RAW/WAW)
     */
    auto *edge = new DGEdge<Instruction>(*oldEdge);

    /*
     * Configure new edge/nodes to point to each other 
     */
    edge->setNodePair(newFromNode, newToNode);
    newPDG->allEdges.push_back(edge);
    connectNodesVia(edge, newFromNode, newToNode);
   }
}