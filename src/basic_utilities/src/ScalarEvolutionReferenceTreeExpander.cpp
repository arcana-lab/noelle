/*
 * This is bits and pieces of LLVM class implementation ScalarEvolutionExpander
 * in an attempt to NOT expand values inline but rather at some arbitrary insertion block
 */

#include "ScalarEvolutionReferencer.hpp"

#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PatternMatch.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

ReferenceTreeExpander::ReferenceTreeExpander (
  SCEVReference *tree,
  std::set<Value *> &valuesToReferenceAndNotExpand,
  IRBuilder<> &expansionBuilder
) : currentNode{tree}, valuesToReferenceAndNotExpand{valuesToReferenceAndNotExpand}, expansionBuilder{expansionBuilder} {

  /*
   * Visit every node in the tree, expanding non-referenced values using the IRBuilder given 
   */
  rootValue = visit(tree->getSCEV());

  /*
   * The current node acts like the top of a stack so must be set back after a call to visit
   */
  currentNode = tree;
}

Value *ReferenceTreeExpander::getRootOfTree () {
  return rootValue;
}

Value *ReferenceTreeExpander::visitConstant (const SCEVConstant *S) {
  return currentNode->getValue();
}

Value *ReferenceTreeExpander::visitTruncateExpr (const SCEVTruncateExpr *S) ;{
  assert(false && "Truncate SCEV expander not implemented");
  return nullptr;
}

Value *ReferenceTreeExpander::visitZeroExtendExpr (const SCEVZeroExtendExpr *S) ;{
  assert(false && "Zero extend SCEV expander not implemented");
  return nullptr;
}

Value *ReferenceTreeExpander::visitSignExtendExpr (const SCEVSignExtendExpr *S) ;{
  assert(false && "Sign extend SCEV expander not implemented");
  return nullptr;
}

Value *ReferenceTreeExpander::visitAddExpr (const SCEVAddExpr *S) ;{
  assert(false && "Add SCEV expander not implemented");
  return nullptr;
}

Value *ReferenceTreeExpander::visitMulExpr (const SCEVMulExpr *S) ;{
  assert(false && "Multiply SCEV expander not implemented");
  return nullptr;
}

Value *ReferenceTreeExpander::visitUDivExpr (const SCEVUDivExpr *S) ;{
  assert(false && "Unsigned divide SCEV expander not implemented");
  return nullptr;
}

Value *ReferenceTreeExpander::visitAddRecExpr (const SCEVAddRecExpr *S) ;{
  assert(valuesToReferenceAndNotExpand.find(currentNode->getValue()) != valuesToReferenceAndNotExpand.end()
    && currentNode->getValue() != nullptr
    && "Expanding add recursive SCEV not implemented; it must be directly referenced");
  return currentNode->getValue();
}

Value *ReferenceTreeExpander::visitSMaxExpr (const SCEVSMaxExpr *S) ;{
  assert(false && "Signed max SCEV expander not implemented");
  return nullptr;
}

Value *ReferenceTreeExpander::visitUMaxExpr (const SCEVUMaxExpr *S) ;{
  assert(false && "Unsigned max SCEV expander not implemented");
  return nullptr;
}

Value *ReferenceTreeExpander::visitSMinExpr (const SCEVSMinExpr *S) ;{
  assert(false && "Signed min SCEV expander not implemented");
  return nullptr;
}

Value *ReferenceTreeExpander::visitUMinExpr (const SCEVUMinExpr *S) ;{
  assert(false && "Unsigned min SCEV expander not implemented");
  return nullptr;
}

Value *ReferenceTreeExpander::visitUnknown (const SCEVUnknown *S) ;{
  assert(valuesToReferenceAndNotExpand.find(currentNode->getValue()) != valuesToReferenceAndNotExpand.end()
    && "Non SCEV value that cannot be expanded is not expected to be referenced either");
  return currentNode->getValue();
}

Value *ReferenceTreeExpander::visitCouldNotCompute (const SCEVCouldNotCompute* S) ;{
  return nullptr;
}
