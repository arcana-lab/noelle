/*
 * Copyright 2016 - 2021  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/IVStepperUtility.hpp"

namespace llvm::noelle{

PHINode *IVUtility::createChunkPHI (BasicBlock *preheaderB, BasicBlock *headerB, Type *chunkPHIType, Value *chunkSize) {

  // TODO: Add asserts to ensure the basic blocks/terminators are well formed

  std::vector<BasicBlock *> headerPreds(pred_begin(headerB), pred_end(headerB));
  IRBuilder<> headerBuilder(headerB->getFirstNonPHIOrDbgOrLifetime());
  auto chunkPHI = headerBuilder.CreatePHI(chunkPHIType, headerPreds.size());
  auto zeroValueForChunking = ConstantInt::get(chunkPHIType, 0);
  auto onesValueForChunking = ConstantInt::get(chunkPHIType, 1);

  for (auto B : headerPreds) {
    IRBuilder<> latchBuilder(B->getTerminator());

    if (preheaderB == B) {
      chunkPHI->addIncoming(zeroValueForChunking, B);
    } else {
      auto chunkIncrement = latchBuilder.CreateAdd(chunkPHI, onesValueForChunking);
      auto isChunkCompleted = latchBuilder.CreateICmp(CmpInst::Predicate::ICMP_EQ, chunkIncrement, chunkSize);
      auto chunkWrap = latchBuilder.CreateSelect(isChunkCompleted, zeroValueForChunking, chunkIncrement, "chunkWrap");
      chunkPHI->addIncoming(chunkWrap, B);
    }
  }

  return chunkPHI;
}

void IVUtility::chunkInductionVariablePHI(
  BasicBlock *preheaderBlock,
  PHINode *ivPHI,
  PHINode *chunkPHI,
  Value *chunkStepSize
) {

  for (auto i = 0; i < ivPHI->getNumIncomingValues(); ++i) {
    auto B = ivPHI->getIncomingBlock(i);
    IRBuilder<> latchBuilder(B->getTerminator());
    if (preheaderBlock == B) continue;

    auto chunkIncomingIdx = chunkPHI->getBasicBlockIndex(B);
    Value *isChunkCompleted = cast<SelectInst>(chunkPHI->getIncomingValue(chunkIncomingIdx))->getCondition();

    auto initialLatchValue = ivPHI->getIncomingValue(i);
    auto ivOffsetByChunk = offsetIVPHI(B, ivPHI, initialLatchValue, chunkStepSize);

    /*
     * Iterate to next chunk if necessary
     */
    ivPHI->setIncomingValue(i, latchBuilder.CreateSelect(
      isChunkCompleted,
      ivOffsetByChunk,
      initialLatchValue, 
      "nextStepOrNextChunk"
    ));

  }
}

void IVUtility::stepInductionVariablePHI (
  BasicBlock *preheaderBlock,
  PHINode *ivPHI,
  Value *additionalStepSize
) {

  for (auto i = 0; i < ivPHI->getNumIncomingValues(); ++i) {
    auto B = ivPHI->getIncomingBlock(i);
    if (preheaderBlock == B) {
      continue;
    }

    auto prevStepRecurrence = ivPHI->getIncomingValue(i);
    auto batchStepRecurrence = offsetIVPHI(B, ivPHI, prevStepRecurrence, additionalStepSize);
    ivPHI->setIncomingValue(i, batchStepRecurrence);
  }

  return ;
}

Value *IVUtility::offsetIVPHI (
  BasicBlock *insertBlock,
  PHINode *ivPHI,
  Value *startValue,
  Value *offsetValue
) {

  IRBuilder<> insertBuilder(insertBlock->getTerminator());
  Value *offsetStartValue = nullptr;

  /*
   * For pointer arithmetic, use ptrtoint - inttoptr paradigm
   */
  auto ivType = ivPHI->getType();
  if (ivType->isPointerTy()) {
    offsetStartValue = insertBuilder.CreateIntToPtr(
      insertBuilder.CreateAdd(
        insertBuilder.CreatePtrToInt(
          startValue,
          offsetValue->getType()
        ),
        offsetValue
      ),
      ivType
    );

  } else {
    offsetStartValue = offsetValue->getType()->isFloatingPointTy() ? insertBuilder.CreateFAdd(startValue, offsetValue) : insertBuilder.CreateAdd(startValue, offsetValue);
  }

  return offsetStartValue;
}

Value *IVUtility::scaleInductionVariableStep (
          BasicBlock *insertBlock,
          PHINode *ivPhi,
          Value *stepSize,
          Value *scale
) {
  IRBuilder<> insertBuilder(insertBlock->getTerminator());

  Value* stepXscale = nullptr;

  auto ivType = ivPhi->getType();
  if (ivType->isFloatingPointTy()) {
    stepXscale = insertBuilder.CreateFMul(
        stepSize,
        insertBuilder.CreateSIToFP( 
          scale,
          stepSize->getType()
          )
      );
  } else {
    stepXscale = insertBuilder.CreateMul(
        stepSize,
        insertBuilder.CreateZExtOrTrunc(
          scale,
          stepSize->getType()
          )
      );    
  }

  return stepXscale;
}

Value *IVUtility::computeInductionVariableValueForIteration (
          BasicBlock *insertBlock,
          PHINode *ivPhi,
          Value *startValue,
          Value *stepSize,
          Value *iteration
) {
  IRBuilder<> insertBuilder(insertBlock->getTerminator());
  Value* stepXiteration = IVUtility::scaleInductionVariableStep(insertBlock, ivPhi, stepSize, iteration);
  Value *valueAtIteration = IVUtility::offsetIVPHI(insertBlock, ivPhi, startValue, stepXiteration);
  return valueAtIteration;
}

}
