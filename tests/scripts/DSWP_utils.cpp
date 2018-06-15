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

#include <ThreadSafeSpinLockQueue.hpp>
#include <ThreadPool.hpp>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>
#include <iostream>

using namespace MARC;

#ifdef DSWP_STATS
static int64_t numberOfPushes8 = 0;
static int64_t numberOfPushes16 = 0;
static int64_t numberOfPushes32 = 0;
static int64_t numberOfPushes64 = 0;
#endif

static ThreadPool pool{std::thread::hardware_concurrency(), true};

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

  void printPushedP(int32_t *p){
    printf("Pushed: %p\n", p);
  }

  void printPulledP(int32_t *p){
    printf("Pulled: %p\n", p);
  }

  void queuePush8(ThreadSafeSpinLockQueue<int8_t> *queue, int8_t *val) { 
    queue->push(*val); 

    #ifdef DSWP_STATS
    numberOfPushes8++;
    #endif

    return ;
  }

  void queuePop8(ThreadSafeSpinLockQueue<int8_t> *queue, int8_t *val) { 
    queue->waitPop(*val); 
    return ;
  }

  void queuePush16(ThreadSafeSpinLockQueue<int16_t> *queue, int16_t *val) { 
    queue->push(*val); 

    #ifdef DSWP_STATS
    numberOfPushes16++;
    #endif

    return ;
  }

  void queuePop16(ThreadSafeSpinLockQueue<int16_t> *queue, int16_t *val) { queue->waitPop(*val); }

  void queuePush32(ThreadSafeSpinLockQueue<int32_t> *queue, int32_t *val) { 
    queue->push(*val); 

    #ifdef DSWP_STATS
    numberOfPushes32++;
    #endif

    return ;
  }

  void queuePop32(ThreadSafeSpinLockQueue<int32_t> *queue, int32_t *val) { queue->waitPop(*val); }

  void queuePush64(ThreadSafeSpinLockQueue<int64_t> *queue, int64_t *val) { 
    queue->push(*val); 

    #ifdef DSWP_STATS
    numberOfPushes64++;
    #endif

    return ;
  }

  void queuePop64(ThreadSafeSpinLockQueue<int64_t> *queue, int64_t *val) { queue->waitPop(*val); }

  void stageExecuter(void (*stage)(void *, void *), void *env, void *queues){ return stage(env, queues); }

  void stageDispatcher(void *env, void *queues, int64_t *queueSizes, void *stages, int64_t numberOfStages, int64_t numberOfQueues){
    #ifdef RUNTIME_PRINT
    std::cerr << "Starting dispatcher: num stages " << numberOfStages << ", num queues: " << numberOfQueues << std::endl;
    #endif

    void *localQueues[numberOfQueues];
    for (auto i = 0; i < numberOfQueues; ++i) {
      switch (queueSizes[i])
      {
        case 1:
          localQueues[i] = new ThreadSafeSpinLockQueue<int8_t>();
          break;
        case 8:
          localQueues[i] = new ThreadSafeSpinLockQueue<int8_t>();
          break;
        case 16:
          localQueues[i] = new ThreadSafeSpinLockQueue<int16_t>();
          break;
        case 32:
          localQueues[i] = new ThreadSafeSpinLockQueue<int32_t>();
          break;
        case 64:
          localQueues[i] = new ThreadSafeSpinLockQueue<int64_t>();
          break;
        default:
          std::cerr << "QUEUE SIZE INCORRECT!\n";
          abort();
          break;
      }
    }
    #ifdef RUNTIME_PRINT
    std::cerr << "Made queues" << std::endl;
    #endif

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
          delete (ThreadSafeSpinLockQueue<int8_t> *)(localQueues[i]);
          break;
        case 8:
          delete (ThreadSafeSpinLockQueue<int8_t> *)(localQueues[i]);
          break;
        case 16:
          delete (ThreadSafeSpinLockQueue<int16_t> *)(localQueues[i]);
          break;
        case 32:
          delete (ThreadSafeSpinLockQueue<int32_t> *)(localQueues[i]);
          break;
        case 64:
          delete (ThreadSafeSpinLockQueue<int64_t> *)(localQueues[i]);
          break;
      }
    }

    #ifdef DSWP_STATS
    std::cout << "DSWP: 1 Byte pushes = " << numberOfPushes8 << std::endl;
    std::cout << "DSWP: 2 Bytes pushes = " << numberOfPushes16 << std::endl;
    std::cout << "DSWP: 4 Bytes pushes = " << numberOfPushes32 << std::endl;
    std::cout << "DSWP: 8 Bytes pushes = " << numberOfPushes64 << std::endl;
    #endif

    return ;
  }
}
