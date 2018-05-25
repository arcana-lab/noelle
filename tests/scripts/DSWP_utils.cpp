#include <future>
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
#include <iostream>

using namespace MARC;

extern "C" {

  typedef void (*stageFunctionPtr_t)(void *, void*);

  void printReachedS(std::string s)
  {
    auto outS = "Reached: " + s;
    printf("%s\n",outS.c_str());
  }

  void printReachedI(int i){
    printf("Reached: %d\n",i);
  }

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
    #ifdef RUNTIME_PRINT
    std::cerr << "Starting dispatcher" << std::endl;
    #endif

    void *localQueues[numberOfQueues];
    for (auto i = 0; i < numberOfQueues; ++i) {
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
    #ifdef RUNTIME_PRINT
    std::cerr << "Made queues" << std::endl;
    #endif

    ThreadPool pool(numberOfStages);
    
    std::vector<MARC::TaskFuture<void>> localFutures;
    auto allStages = (void **)stages;
    for (auto i = 0; i < numberOfStages; ++i) {
      auto stagePtr = reinterpret_cast<stageFunctionPtr_t>(reinterpret_cast<long long>(allStages[i]));
      localFutures.push_back(pool.submit(stagePtr, env, (void*)localQueues));
      #ifdef RUNTIME_PRINT
      std::cerr << "Submitted stage" << std::endl;
      #endif
    }
    #ifdef RUNTIME_PRINT
    std::cerr << "Submitted pool" << std::endl;
    #endif

    for (auto& future : localFutures){
      future.get();
    }
    #ifdef RUNTIME_PRINT
    std::cerr << "Got all futures" << std::endl;
    #endif

    for (int i = 0; i < numberOfQueues; ++i) {
      switch (queueSizes[i]) {
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

    return ;
  }
}
