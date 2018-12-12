#include "HELIX.hpp"
#include "HELIXTask.hpp"

using namespace llvm ;

void HELIX::addChunkFunctionExecutionAsideOriginalLoop (
  LoopDependenceInfo *LDI,
  Parallelization &par,
  uint64_t numberOfSequentialSegments
) {

  /*
   * Create the entry and exit points of the function that will include the parallelized loop.
   */
  auto &cxt = LDI->function->getContext();
  LDI->entryPointOfParallelizedLoop = BasicBlock::Create(cxt, "", LDI->function);
  LDI->exitPointOfParallelizedLoop = BasicBlock::Create(cxt, "", LDI->function);

  /*
   * Create the environment.
   * This will append store instructions to LDI->entryPointOfParallelizedLoop to initialize the environment array.
   */
  this->allocateEnvironmentArray(LDI);
  this->populateLiveInEnvironment(LDI);

  /*
   * Fetch the pointer to the environment.
   */
  auto envPtr = envBuilder->getEnvArrayInt8Ptr();

  /*
   * Fetch the number of cores
   */
  auto numCores = ConstantInt::get(par.int64, LDI->maximumNumberOfCoresForTheParallelization);

  /*
   * Fetch the chunk size.
   */
  auto numOfSS = ConstantInt::get(par.int64, numberOfSequentialSegments);

  /*
   * Call the function that incudes the parallelized loop.
   */
  IRBuilder<> helixBuilder(LDI->entryPointOfParallelizedLoop);
  helixBuilder.CreateCall(this->taskDispatcher, ArrayRef<Value *>({
    (Value *)tasks[0]->F,
    envPtr,
    numCores,
    numOfSS
  }));

  /*
   * Propagate the last value of live-out variables to the code outside the parallelized loop.
   */
  this->propagateLiveOutEnvironment(LDI);

  /*
   * Jump to the unique successor of the loop.
   */
  helixBuilder.CreateBr(LDI->exitPointOfParallelizedLoop);

  return ;
}
