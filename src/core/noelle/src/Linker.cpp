/*
 * Copyright 2021 - 2022  Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "noelle/core/Linker.hpp"
#include "noelle/core/Architecture.hpp"

namespace llvm::noelle {

Linker::Linker(Module &M, TypesManager *tm)
  : program{M},
    tm{tm}{

  return ;
}

void Linker::linkTransformedLoopToOriginalFunction(
    BasicBlock *originalPreHeader,
    BasicBlock *startOfParLoopInOriginalFunc,
    BasicBlock *endOfParLoopInOriginalFunc,
    Value *envArray,
    Value *envIndexForExitVariable,
    std::vector<BasicBlock *> &loopExitBlocks) {

  /*
   * Create the global variable for the parallelized loop.
   */
  auto integerType = this->tm->getIntegerType(32);
  auto globalBool = new GlobalVariable(this->program,
                                       integerType,
                                       /*isConstant=*/false,
                                       GlobalValue::ExternalLinkage,
                                       Constant::getNullValue(integerType));
  auto const0 = ConstantInt::get(integerType, 0);
  auto const1 = ConstantInt::get(integerType, 1);

  /*
   * Fetch the terminator of the preheader.
   */
  auto originalTerminator = originalPreHeader->getTerminator();

  /*
   * Fetch the header of the original loop.
   */
  auto originalHeader = originalTerminator->getSuccessor(0);

  /*
   * Check if another invocation of the loop is running in parallel.
   */
  IRBuilder<> loopSwitchBuilder(originalTerminator);
  auto globalLoad = loopSwitchBuilder.CreateLoad(globalBool);
  auto compareInstruction = loopSwitchBuilder.CreateICmpEQ(globalLoad, const0);
  loopSwitchBuilder.CreateCondBr(compareInstruction,
                                 startOfParLoopInOriginalFunc,
                                 originalHeader);
  originalTerminator->eraseFromParent();

  IRBuilder<> endBuilder(endOfParLoopInOriginalFunc);

  /*
   * Load exit block environment variable and branch to the correct loop exit
   * block
   */
  if (loopExitBlocks.size() == 1) {
    endBuilder.CreateBr(loopExitBlocks[0]);

  } else {

    /*
     * Compute how many values can fit in a cache line.
     */
    auto valuesInCacheLine =
        Architecture::getCacheLineBytes() / sizeof(int64_t);

    auto int64 = this->tm->getIntegerType(64);
    auto exitEnvPtr = endBuilder.CreateInBoundsGEP(
        envArray,
        ArrayRef<Value *>({ cast<Value>(ConstantInt::get(int64, 0)),
                            endBuilder.CreateMul(
                                envIndexForExitVariable,
                                ConstantInt::get(int64, valuesInCacheLine)) }));
    auto exitEnvCast =
        endBuilder.CreateIntCast(endBuilder.CreateLoad(exitEnvPtr),
                                 integerType,
                                 /*isSigned=*/false);
    auto exitSwitch = endBuilder.CreateSwitch(exitEnvCast, loopExitBlocks[0]);
    for (int i = 1; i < loopExitBlocks.size(); ++i) {
      auto constantInt = cast<ConstantInt>(ConstantInt::get(integerType, i));
      exitSwitch->addCase(constantInt, loopExitBlocks[i]);
    }
  }

  /*
   * NOTE(angelo): LCSSA constants need to be replicated for parallelized code
   * path
   */
  for (auto bb : loopExitBlocks) {
    for (auto &I : *bb) {
      if (auto phi = dyn_cast<PHINode>(&I)) {
        auto bbIndex = phi->getBasicBlockIndex(originalHeader);
        if (bbIndex == -1) {
          continue;
        }
        auto val = phi->getIncomingValue(bbIndex);
        if (isa<Constant>(val)) {
          phi->addIncoming(val, endOfParLoopInOriginalFunc);
        }
        continue;
      }
      break;
    }
  }

  /*
   * Set/Reset global variable so only one invocation of the loop is run in
   * parallel at a time.
   */
  if (startOfParLoopInOriginalFunc == endOfParLoopInOriginalFunc) {
    endBuilder.SetInsertPoint(&*endOfParLoopInOriginalFunc->begin());
    endBuilder.CreateStore(const1, globalBool);
  } else {
    IRBuilder<> startBuilder(&*startOfParLoopInOriginalFunc->begin());
    startBuilder.CreateStore(const1, globalBool);
  }
  endBuilder.SetInsertPoint(endOfParLoopInOriginalFunc->getTerminator());
  endBuilder.CreateStore(const0, globalBool);

  return;
}

void Linker::substituteOriginalLoopWithTransformedLoop(
    BasicBlock *originalPreHeader,
    BasicBlock *startOfParLoopInOriginalFunc,
    BasicBlock *endOfParLoopInOriginalFunc,
    Value *envArray,
    Value *envIndexForExitVariable,
    std::vector<BasicBlock *> &loopExitBlocks) {

  /*
   * Fetch the terminator of the preheader.
   */
  auto originalTerminator = originalPreHeader->getTerminator();

  /*
   * Fetch the header of the original loop.
   */
  auto originalHeader = originalTerminator->getSuccessor(0);

  /*
   * Check if another invocation of the loop is running in parallel.
   */
  IRBuilder<> loopSwitchBuilder(originalTerminator);
  loopSwitchBuilder.CreateBr(startOfParLoopInOriginalFunc);
  originalTerminator->eraseFromParent();

  /*
   * Load exit block environment variable and branch to the correct loop exit
   * block
   */
  IRBuilder<> endBuilder(endOfParLoopInOriginalFunc);
  if (loopExitBlocks.size() == 1) {
    endBuilder.CreateBr(loopExitBlocks[0]);

  } else {

    /*
     * Compute how many values can fit in a cache line.
     */
    auto valuesInCacheLine =
        Architecture::getCacheLineBytes() / sizeof(int64_t);

    auto int64 = this->tm->getIntegerType(64);
    auto exitEnvPtr = endBuilder.CreateInBoundsGEP(
        envArray,
        ArrayRef<Value *>({ cast<Value>(ConstantInt::get(int64, 0)),
                            endBuilder.CreateMul(
                                envIndexForExitVariable,
                                ConstantInt::get(int64, valuesInCacheLine)) }));
  auto integerType = this->tm->getIntegerType(32);
    auto exitEnvCast =
        endBuilder.CreateIntCast(endBuilder.CreateLoad(exitEnvPtr),
                                 integerType,
                                 /*isSigned=*/false);
    auto exitSwitch = endBuilder.CreateSwitch(exitEnvCast, loopExitBlocks[0]);
    for (int i = 1; i < loopExitBlocks.size(); ++i) {
      auto constantInt = cast<ConstantInt>(ConstantInt::get(integerType, i));
      exitSwitch->addCase(constantInt, loopExitBlocks[i]);
    }
  }

  /*
   * LCSSA constants need to be replicated for parallelized code path
   */
  for (auto bb : loopExitBlocks) {
    for (auto &I : *bb) {
      if (auto phi = dyn_cast<PHINode>(&I)) {
        auto bbIndex = phi->getBasicBlockIndex(originalHeader);
        if (bbIndex == -1) {
          continue;
        }
        auto val = phi->getIncomingValue(bbIndex);
        if (isa<Constant>(val)) {
          phi->addIncoming(val, endOfParLoopInOriginalFunc);
        }
        continue;
      }
      break;
    }
  }

  return;
}

}
