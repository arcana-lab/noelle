#include "DSWP.hpp"

using namespace llvm;

bool Parallelizer::collectThreadPoolHelperFunctionsAndTypes (Module &M, Parallelization &par) {
  printReachedI = M.getFunction("printReachedI");
  printPushedP = M.getFunction("printPushedP");
  printPulledP = M.getFunction("printPulledP");
  std::string pushers[4] = { "queuePush8", "queuePush16", "queuePush32", "queuePush64" };
  std::string poppers[4] = { "queuePop8", "queuePop16", "queuePop32", "queuePop64" };
  for (auto pusher : pushers) queuePushes.push_back(M.getFunction(pusher));
  for (auto popper : poppers) queuePops.push_back(M.getFunction(popper));
  for (auto queueF : queuePushes) queueTypes.push_back(queueF->arg_begin()->getType());
  queueSizeToIndex = unordered_map<int, int>({ { 1, 0 }, { 8, 0 }, { 16, 1 }, { 32, 2 }, { 64, 3 }});
  queueElementTypes = std::vector<Type *>({ par.int8, par.int16, par.int32, par.int64 });

  stageDispatcher = M.getFunction("stageDispatcher");
  auto stageExecuter = M.getFunction("stageExecuter");

  auto stageArgType = stageExecuter->arg_begin()->getType();
  stageType = cast<FunctionType>(cast<PointerType>(stageArgType)->getElementType());
  return true;
}
