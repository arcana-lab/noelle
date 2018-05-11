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

extern "C" {

  // void printReachedI(int i){
  //   printf("%d\n",i);
  // }

  void queuePush8(ThreadSafeQueue<int8_t> *queue, int8_t *val) { queue->push(*val); }
  void queuePop8(ThreadSafeQueue<int8_t> *queue, int8_t *val) { queue->waitPop(*val); }
  void queuePush16(ThreadSafeQueue<int16_t> *queue, int16_t *val) { queue->push(*val); }
  void queuePop16(ThreadSafeQueue<int16_t> *queue, int16_t *val) { queue->waitPop(*val); }
  void queuePush32(ThreadSafeQueue<int32_t> *queue, int32_t *val) { queue->push(*val); }
  void queuePop32(ThreadSafeQueue<int32_t> *queue, int32_t *val) { queue->waitPop(*val); }
  void queuePush64(ThreadSafeQueue<int64_t> *queue, int64_t *val) { queue->push(*val); }
  void queuePop64(ThreadSafeQueue<int64_t> *queue, int64_t *val) { queue->waitPop(*val); }

  void queuePush(ThreadSafeQueue<char> *queue, char *val, int64_t byteLength){
    char *oldVal = val;
    for (int i = 0; i < byteLength; ++i, ++val) 
    {
      queue->push(*val);
    }
    val = oldVal;
  }

  void queuePop(ThreadSafeQueue<char> *queue, char *val, int64_t byteLength){
    char *oldVal = val;
    for (int i = 0; i < byteLength; ++i, ++val)
    {
      while (!queue->waitPop(*val)) printf("Spurious pop\n");
    }
    val = oldVal;
  }

  void stageExecuter(void (*stage)(void *, void *), void *env, void *queues){ return stage(env, queues); }

  void stageDispatcher(void *env, void *queues, int64_t *queueSizes, void *stages, int64_t numberOfStages, int64_t numberOfQueues){
    void *localQueues[numberOfQueues];
    for (int i = 0; i < numberOfQueues; ++i)
    {
      switch (queueSizes[i])
      {
        case 1:
          localQueues[i] = new ThreadSafeQueue<int8_t>();
          break;
        case 8:
          localQueues[i] = new ThreadSafeQueue<int8_t>();
          break;
        case 16:
          localQueues[i] = new ThreadSafeQueue<int16_t>();
          break;
        case 32:
          localQueues[i] = new ThreadSafeQueue<int32_t>();
          break;
        case 64:
          localQueues[i] = new ThreadSafeQueue<int64_t>();
          break;
      }
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
      switch (queueSizes[i])
      {
        case 1:
          delete (ThreadSafeQueue<int8_t> *)(localQueues[i]);
          break;
        case 8:
          delete (ThreadSafeQueue<int8_t> *)(localQueues[i]);
          break;
        case 16:
          delete (ThreadSafeQueue<int16_t> *)(localQueues[i]);
          break;
        case 32:
          delete (ThreadSafeQueue<int32_t> *)(localQueues[i]);
          break;
        case 64:
          delete (ThreadSafeQueue<int64_t> *)(localQueues[i]);
          break;
      }
    }
  }
}