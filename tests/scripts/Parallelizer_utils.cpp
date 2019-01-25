#include <future>
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <pthread.h>
#include <functional>
#include <memory>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <ThreadSafeQueue.hpp>
#include <ThreadSafeLockFreeQueue.hpp>
#include <ThreadPool.hpp>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>
#include <iostream>

using namespace MARC;

#define CACHE_LINE_SIZE 64

#ifdef DSWP_STATS
static int64_t numberOfPushes8 = 0;
static int64_t numberOfPushes16 = 0;
static int64_t numberOfPushes32 = 0;
static int64_t numberOfPushes64 = 0;
#endif

static ThreadPool pool{true, std::thread::hardware_concurrency()};

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

  void queuePush8(ThreadSafeQueue<int8_t> *queue, int8_t *val) { 
    queue->push(*val); 

    #ifdef DSWP_STATS
    numberOfPushes8++;
    #endif

    return ;
  }

  void queuePop8(ThreadSafeQueue<int8_t> *queue, int8_t *val) { 
    queue->waitPop(*val); 
    return ;
  }

  void queuePush16(ThreadSafeQueue<int16_t> *queue, int16_t *val) { 
    queue->push(*val); 

    #ifdef DSWP_STATS
    numberOfPushes16++;
    #endif

    return ;
  }

  void queuePop16(ThreadSafeQueue<int16_t> *queue, int16_t *val) { 
    queue->waitPop(*val);
  }

  void queuePush32(ThreadSafeQueue<int32_t> *queue, int32_t *val) { 
    queue->push(*val); 

    #ifdef DSWP_STATS
    numberOfPushes32++;
    #endif

    return ;
  }

  void queuePop32(ThreadSafeQueue<int32_t> *queue, int32_t *val) { 
    queue->waitPop(*val);
  }

  void queuePush64(ThreadSafeQueue<int64_t> *queue, int64_t *val) { 
    queue->push(*val); 

    #ifdef DSWP_STATS
    numberOfPushes64++;
    #endif

    return ;
  }

  void queuePop64(ThreadSafeQueue<int64_t> *queue, int64_t *val) { 
    queue->waitPop(*val); 

    return ;
  }

  void stageExecuter(void (*stage)(void *, void *), void *env, void *queues){ 
    return stage(env, queues);
  }

  void doallDispatcher (void (*chunker)(void *, int64_t, int64_t, int64_t), void *env, int64_t numCores, int64_t chunkSize){
    #ifdef RUNTIME_PRINT
    std::cerr << "Starting dispatcher: num cores " << numCores << ", chunk size: " << chunkSize << std::endl;
    #endif

    std::vector<MARC::TaskFuture<void>> localFutures;
    for (auto i = 0; i < numCores; ++i) {
      localFutures.push_back(pool.submit(chunker, env, i, numCores, chunkSize));
      #ifdef RUNTIME_PRINT
      std::cerr << "Submitted chunker on core " << i << std::endl;
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

    return ;
  }

  void HELIX_dispatcher (
    void (*parallelizedLoop)(void *, void *, void *, int64_t, int64_t), 
    void *env,
    int64_t numCores, 
    int64_t numOfsequentialSegments
    ){

    /*
     * Assumptions.
     */
    assert(parallelizedLoop != NULL);
    assert(env != NULL);
    assert(numCores > 1);
    assert(numOfsequentialSegments > 0);

    /*
     * Allocate the sequential segment arrays.
     * We need numCores - 1 arrays.
     */
    auto numOfSSArrays = numCores - 1;
    void *ssArrays = NULL;
    auto ssSize = CACHE_LINE_SIZE;
    auto ssArraySize = ssSize * numOfsequentialSegments;
    posix_memalign(&ssArrays, CACHE_LINE_SIZE, ssArraySize *  numOfSSArrays);
    if (ssArrays == NULL){
      fprintf(stderr, "HelixDispatcher: ERROR = not enough memory to allocate %lld sequential segment arrays\n", (long long)numCores);
      abort();
    }

    /*
     * Initialize the sequential segment arrays.
     */
    for (auto i = 0; i < numOfSSArrays; i++){

      /*
       * Fetch the current sequential segment array.
       */
      auto ssArray = (void *)(((uint64_t)ssArrays) + (i * ssArraySize));

      /*
       * Initialize the locks.
       */
      for (auto lockID = 0; lockID < numOfsequentialSegments; lockID++){

        /*
         * Fetch the pointer to the current lock.
         */
        auto lock = (pthread_spinlock_t *)(((uint64_t)ssArray) + (lockID * ssSize));

        /*
         * Initialize the lock.
         */
        pthread_spin_init(lock, PTHREAD_PROCESS_PRIVATE);
        
        /*
         * If the sequential segment is not for core 0, then we need to lock it.
         */
        if (i > 0){
          pthread_spin_lock(lock);
        }
      }
    }

    /*
     * Launch threads
     */
    std::vector<MARC::TaskFuture<void>> localFutures;
    for (auto i = 0; i < numCores; ++i) {

      /*
       * Identify the past and future sequential segment arrays.
       */
      auto pastID = i;
      auto futureID = (i + 1) % numCores;

      /*
       * Fetch the sequential segment array for the current thread.
       */
      auto ssArrayPast = (void *)(((uint64_t)ssArrays) + (pastID * ssArraySize));
      auto ssArrayFuture = (void *)(((uint64_t)ssArrays) + (futureID * ssArraySize));
      assert(ssArrayPast != ssArrayFuture);

      /*
       * Launch the thread.
       */
      localFutures.push_back(pool.submit(parallelizedLoop, env, ssArrayPast, ssArrayFuture, i, numCores));

      /*
       * Launch the helper thread.
       */
      //TODO
    }

    /*
     * Wait for the threads to end
     */
    for (auto& future : localFutures){
      future.get();
    }

    /*
     * Free the memory.
     */
    free(ssArrays);

    return ;
  }

  void HELIX_wait (
    void *sequentialSegment
    ){

    /*
     * Fetch the spinlock
     */
    auto ss = (pthread_spinlock_t *) sequentialSegment;
    assert(ss != NULL);

    /*
     * Wait
     */
    pthread_spin_lock(ss);

    return ;
  }

  void HELIX_signal (
    void *sequentialSegment
    ){

    /*
     * Fetch the spinlock
     */
    auto ss = (pthread_spinlock_t *) sequentialSegment;
    assert(ss != NULL);

    /*
     * Signal
     */
    pthread_spin_unlock(ss);

    return ;
  }

  void stageDispatcher (void *env, int64_t *queueSizes, void *stages, int64_t numberOfStages, int64_t numberOfQueues){
    #ifdef RUNTIME_PRINT
    std::cerr << "Starting dispatcher: num stages " << numberOfStages << ", num queues: " << numberOfQueues << std::endl;
    #endif

    void *localQueues[numberOfQueues];
    for (auto i = 0; i < numberOfQueues; ++i) {
      switch (queueSizes[i]) {
        case 1:
          localQueues[i] = new ThreadSafeLockFreeQueue<int8_t>();
          break;
        case 8:
          localQueues[i] = new ThreadSafeLockFreeQueue<int8_t>();
          break;
        case 16:
          localQueues[i] = new ThreadSafeLockFreeQueue<int16_t>();
          break;
        case 32:
          localQueues[i] = new ThreadSafeLockFreeQueue<int32_t>();
          break;
        case 64:
          localQueues[i] = new ThreadSafeLockFreeQueue<int64_t>();
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
          delete (ThreadSafeLockFreeQueue<int8_t> *)(localQueues[i]);
          break;
        case 8:
          delete (ThreadSafeLockFreeQueue<int8_t> *)(localQueues[i]);
          break;
        case 16:
          delete (ThreadSafeLockFreeQueue<int16_t> *)(localQueues[i]);
          break;
        case 32:
          delete (ThreadSafeLockFreeQueue<int32_t> *)(localQueues[i]);
          break;
        case 64:
          delete (ThreadSafeLockFreeQueue<int64_t> *)(localQueues[i]);
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
