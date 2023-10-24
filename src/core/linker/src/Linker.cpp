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

Linker::Linker(Module &M, TypesManager *tm) : program{ M }, tm{ tm } {

  return;
}

void Linker::linkTransformedLoopToOriginalFunction(
    BasicBlock *originalPreHeader,
    BasicBlock *startOfParLoopInOriginalFunc,
    BasicBlock *endOfParLoopInOriginalFunc,
    Value *envArray,
    Value *envIndexForExitVariable,
    std::vector<BasicBlock *> &loopExitBlocks,
    uint32_t minIdleCores) {

  /*
   * Fetch the runtime API to invoke.
   */
  auto coreChecker = this->program.getFunction("NOELLE_getAvailableCores");
  assert(coreChecker != nullptr);

  /*
   * Create the constants.
   */
  auto integerType = this->tm->getIntegerType(32);
  auto minIdleCoresValue = ConstantInt::get(integerType, minIdleCores);

  /*
   * Fetch the terminator of the preheader.
   */
  auto originalTerminator = originalPreHeader->getTerminator();

  /*
   * Fetch the header of the original loop.
   */
  auto originalHeader = originalTerminator->getSuccessor(0);

  /*
   * Check if there are enough idle cores.
   */
  IRBuilder<> loopSwitchBuilder(originalTerminator);
  auto callToCoreChecker =
      loopSwitchBuilder.CreateCall(coreChecker->getFunctionType(), coreChecker);
  auto compareInstruction =
      loopSwitchBuilder.CreateICmpUGE(callToCoreChecker, minIdleCoresValue);
  loopSwitchBuilder.CreateCondBr(compareInstruction,
                                 startOfParLoopInOriginalFunc,
                                 originalHeader);
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
    auto exitEnvCast =
        endBuilder.CreateIntCast(endBuilder.CreateLoad(exitEnvPtr),
                                 integerType,
                                 /*isSigned=*/false);
    auto exitSwitch = endBuilder.CreateSwitch(exitEnvCast, loopExitBlocks[0]);
    for (auto i = 1u; i < loopExitBlocks.size(); ++i) {
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

  return;
}

void Linker::substituteOriginalLoopWithTransformedLoop(
    LoopStructure *originalLoop,
    BasicBlock *startOfParLoopInOriginalFunc,
    BasicBlock *endOfParLoopInOriginalFunc,
    Value *envArray,
    Value *envIndexForExitVariable,
    std::vector<BasicBlock *> &loopExitBlocks,
    uint32_t minIdleCores) {

  /*
   * Fetch the terminator of the preheader.
   */
  auto originalPreHeader = originalLoop->getPreHeader();
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
    for (auto i = 1u; i < loopExitBlocks.size(); ++i) {
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
        phi->removeIncomingValue(bbIndex);
        continue;
      }
      break;
    }
  }

  /*
   * Delete the original loop.
   */
  for (auto bb : originalLoop->getBasicBlocks()) {
    bb->eraseFromParent();
  }

  return;
}

} // namespace llvm::noelle
