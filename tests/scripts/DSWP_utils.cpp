#include <stdio.h>
#include <future>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <ThreadSafeQueue.hpp>
#include <ThreadPool.hpp>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

using namespace MARC;

/*
extern "C" void printReachedIter(int iter){
  printf("Iter:\t%d\n", iter);
}
*/
extern "C" void queuePush(ThreadSafeQueue<char> *queue, char *val, int64_t byteLength){
  char *oldVal = val;
  for (int i = 0; i < byteLength; ++i, ++val) 
  {
    queue->push(*val);
  }
  val = oldVal;
}

extern "C" void queuePop(ThreadSafeQueue<char> *queue, char *val, int64_t byteLength){
  char *oldVal = val;
  for (int i = 0; i < byteLength; ++i, ++val)
  {
    while (!queue->waitPop(*val)) printf("Spurious pop\n");
  }
  val = oldVal;
}

extern "C" void stageExecuter(void (*stage)(void *, void *), void *env, void *queues){ return stage(env, queues); }

extern "C" void stageDispatcher(void *env, void *queues, void *stages, int64_t numberOfStages, int64_t numberOfQueues){
  ThreadSafeQueue<char> *localQueues[numberOfQueues];
  for (int i = 0; i < numberOfQueues; ++i)
  {
    localQueues[i] = new ThreadSafeQueue<char>();
  }
  queues = localQueues;

  ThreadPool pool(numberOfStages);
  auto localFutures = (TaskFuture<void> *)malloc(numberOfStages * sizeof(TaskFuture<void>));
  for (int i = 0; i < numberOfStages; ++i)
  {
    auto stage = ((void (**)(void *, void *)) stages)[i];
    localFutures[i] = std::move(pool.submit(stage, env, queues));
  }

  for (int i = 0; i < numberOfStages; ++i)
  {
    localFutures[i].get();
  }

  for (int i = 0; i < numberOfQueues; ++i)
  {
    delete localQueues[i];
  }
}
