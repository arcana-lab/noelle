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

void HELIX_helperThread (void *ssArray, uint32_t numOfsequentialSegments, uint64_t *theLoopIsOver){

  while ((*theLoopIsOver) == 0){

    /*
     * Prefetch all sequential segment cache lines of the current loop iteration.
     */
    for (auto i = 0 ; ((*theLoopIsOver) == 0) && (i < numOfsequentialSegments); i++){

      /*
       * Fetch the pointer.
       */
      auto ptr = (uint64_t *)(((uint64_t)ssArray) + (i * CACHE_LINE_SIZE));

      /*
       * Prefetch the cache line for the current sequential segment.
       */
      while (((*theLoopIsOver) == 0) && ((*ptr) == 0)) ;
    }
  }

  return ;
}
    
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

  #ifdef RUNTIME_PRINT
  void *mySSGlobal = nullptr;
  #endif

  void HELIX_dispatcher (
    void (*parallelizedLoop)(void *, void *, void *, void *, int64_t, int64_t, uint64_t *), 
    void *env,
    void *loopCarriedArray,
    int64_t numCores, 
    int64_t numOfsequentialSegments
    ){
    #ifdef RUNTIME_PRINT
    std::cerr << "HELIX: dispatcher: Start" << std::endl;
    std::cerr << "HELIX: dispatcher:  Number of sequential segments = " << numOfsequentialSegments << std::endl;
    std::cerr << "HELIX: dispatcher:  Number of cores = " << numCores << std::endl;
    #endif

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
    auto numOfSSArrays = numCores;
    void *ssArrays = NULL;
    auto ssSize = CACHE_LINE_SIZE;
    auto ssArraySize = ssSize * numOfsequentialSegments;
    posix_memalign(&ssArrays, CACHE_LINE_SIZE, ssArraySize *  numOfSSArrays);

    #ifdef RUNTIME_PRINT
    mySSGlobal = ssArrays;
    #endif

    if (ssArrays == NULL){
      fprintf(stderr, "HELIX: dispatcher: ERROR = not enough memory to allocate %lld sequential segment arrays\n", (long long)numCores);
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
    uint64_t loopIsOverFlag = 0;
    cpu_set_t cores;
    std::vector<MARC::TaskFuture<void>> localFutures;
    for (auto i = 0; i < numCores; ++i) {
      #ifdef RUNTIME_PRINT
      fprintf(stderr, "HelixDispatcher: Creating future for core %d\n", i);
      #endif

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

      #ifdef RUNTIME_PRINT
      fprintf(stderr, "HelixDispatcher: defined ss past and future arrays: %ld %ld\n", (int *)ssArrayPast - (int *)mySSGlobal, (int *)ssArrayFuture - (int *)mySSGlobal);
      #endif

      /*
       * Set the affinity for both the thread and its helper.
       */
      CPU_ZERO(&cores);
      auto physicalCore = i * 2;
      CPU_SET(physicalCore, &cores);
      CPU_SET(physicalCore + 1, &cores);

      /*
       * Launch the thread.
       */
      localFutures.push_back(pool.submitToCores(
        cores,
        parallelizedLoop,
        env, loopCarriedArray,
        ssArrayPast, ssArrayFuture,
        i, numCores,
        &loopIsOverFlag
      ));

      /*
       * Launch the helper thread.
       */
      continue ;
      localFutures.push_back(pool.submitToCores(
        cores,
        HELIX_helperThread, 
        ssArrayPast,
        numOfsequentialSegments,
        &loopIsOverFlag
      ));
    }

    #ifdef RUNTIME_PRINT
    std::cerr << "Submitted pool\n";
    int futureGotten = 0;
    #endif

    /*
     * Wait for the threads to end
     */
    for (auto& future : localFutures){
      future.get();

      #ifdef RUNTIME_PRINT
      fprintf(stderr, "Got future: %d\n", futureGotten++);
      #endif
    }

    #ifdef RUNTIME_PRINT
    std::cerr << "Got all futures\n";
    #endif

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

    #ifdef RUNTIME_PRINT
    fprintf(stderr, "HelixDispatcher: Waiting on sequential segment: %ld\n", (int *)sequentialSegment - (int *)mySSGlobal);
    #endif

    /*
     * Wait
     */
    pthread_spin_lock(ss);

    #ifdef RUNTIME_PRINT
    fprintf(stderr, "HelixDispatcher: Waited on sequential segment: %ld\n", (int *)sequentialSegment - (int *)mySSGlobal);
    #endif

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

    #ifdef RUNTIME_PRINT
    fprintf(stderr, "HelixDispatcher: Signaling on sequential segment: %ld\n", (int *)sequentialSegment - (int *)mySSGlobal);
    #endif

    /*
     * Signal
     */
    pthread_spin_unlock(ss);

    #ifdef RUNTIME_PRINT
    fprintf(stderr, "HelixDispatcher: Signaled on sequential segment: %ld\n", (int *)sequentialSegment - (int *)mySSGlobal);
    #endif

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
