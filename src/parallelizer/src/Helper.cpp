/*
 * Copyright 2016 - 2019  Angelo Matni, Simone Campanoni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "Parallelizer.hpp"

using namespace llvm;

bool Parallelizer::collectThreadPoolHelperFunctionsAndTypes (Module &M, Noelle &par) {
  std::string pushers[4] = { "queuePush8", "queuePush16", "queuePush32", "queuePush64" };
  std::string poppers[4] = { "queuePop8", "queuePop16", "queuePop32", "queuePop64" };
  for (auto pusher : pushers) {
    auto pushFunction = M.getFunction(pusher);
    if (pushFunction == nullptr){
      errs() << "Parallelizer: ERROR = function \"" << pusher << "\" could not be found\n";
      abort();
    }
    par.queues.queuePushes.push_back(pushFunction);
  }
  for (auto popper : poppers) {
    auto popFunction = M.getFunction(popper);
    if (popFunction == nullptr){
      errs() << "Parallelizer: ERROR = function \"" << popper << "\" could not be found\n";
      abort();
    }
    par.queues.queuePops.push_back(popFunction);
  }
  for (auto queueF : par.queues.queuePushes) {
    par.queues.queueTypes.push_back(queueF->arg_begin()->getType());
  }
  par.queues.queueSizeToIndex = unordered_map<int, int>({ { 1, 0 }, { 8, 0 }, { 16, 1 }, { 32, 2 }, { 64, 3 }});
  par.queues.queueElementTypes = std::vector<Type *>({ par.int8, par.int16, par.int32, par.int64 });

  return true;
}
