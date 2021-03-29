#include <stdlib.h>

#include <sched.h>

#include "rt/virgil/virgil.h"
#include "rt/virgil/ThreadSafeQueue.hpp"
#include "rt/virgil/ThreadSafeNautilusQueue.hpp"

#define CACHE_LINE_SIZE 64

/*
 * OPTIONS
 */
//#define RUNTIME_PROFILE
//#define RUNTIME_PRINT

typedef struct {
  void (*parallelizedLoop)(void *, int64_t, int64_t, int64_t) ;
  void *env ;
  int64_t coreID ;
  int64_t numCores;
  int64_t chunkSize ;
} DOALL_args_t ;

class NoelleRuntime {
  public:
    NoelleRuntime ();

    uint32_t reserveCores (uint32_t coresRequested);

    void releaseCores (uint32_t coresReleased);

    DOALL_args_t * getDOALLArgs (uint32_t cores, uint32_t *index, nk_virgil_task_t **taskIDs);

    void releaseDOALLArgs (uint32_t index);

  private:
    mutable nk_virgil_spinlock_t doallMemoryLock;
    std::vector<uint32_t> doallMemorySizes;
    std::vector<bool> doallMemoryAvailability;
    std::vector<DOALL_args_t *> doallMemory;
    nk_virgil_task_t **doallMemoryTasks;
    uint32_t doallMemoryChunks;

    uint32_t getMaximumNumberOfCores (void);

    /*
     * Current number of idle cores.
     */
    int32_t NOELLE_idleCores;

    /*
     * Maximum number of cores.
     */
    uint32_t maxCores;

    mutable nk_virgil_spinlock_t spinLock;
};

#ifdef RUNTIME_PROFILE
nk_virgil_spinlock_t printLock;
uint64_t clocks_starts[64];
uint64_t clocks_ends[64];
#endif

static NoelleRuntime runtime{};

extern "C" {

  /******************************************** NOELLE APIs ***********************************************/

  class DispatcherInfo {
    public:
      int32_t numberOfThreadsUsed;
      int64_t unusedVariableToPreventOptIfStructHasOnlyOneVariable;
  };

  /*
   * Dispatch threads to run a DOALL loop.
   */
  DispatcherInfo NOELLE_DOALLDispatcher (
    void (*parallelizedLoop)(void *, int64_t, int64_t, int64_t), 
    void *env, 
    int64_t maxNumberOfCores, 
    int64_t chunkSize
    );


  /******************************************** NOELLE API implementations ***********************************************/

  typedef void (*stageFunctionPtr_t)(void *, void*);

  void queuePush8(MARC::ThreadSafeQueue<int8_t> *queue, int8_t *val) { 
    queue->push(*val); 

    #ifdef DSWP_STATS
    numberOfPushes8++;
    #endif

    return ;
  }

  void queuePop8(MARC::ThreadSafeQueue<int8_t> *queue, int8_t *val) { 
    queue->waitPop(*val); 
    return ;
  }

  void queuePush16(MARC::ThreadSafeQueue<int16_t> *queue, int16_t *val) { 
    queue->push(*val); 

    #ifdef DSWP_STATS
    numberOfPushes16++;
    #endif

    return ;
  }

  void queuePop16(MARC::ThreadSafeQueue<int16_t> *queue, int16_t *val) { 
    queue->waitPop(*val);
  }

  void queuePush32(MARC::ThreadSafeQueue<int32_t> *queue, int32_t *val) { 
    queue->push(*val); 

    #ifdef DSWP_STATS
    numberOfPushes32++;
    #endif

    return ;
  }

  void queuePop32(MARC::ThreadSafeQueue<int32_t> *queue, int32_t *val) { 
    queue->waitPop(*val);
  }

  void queuePush64(MARC::ThreadSafeQueue<int64_t> *queue, int64_t *val) { 
    queue->push(*val); 

    #ifdef DSWP_STATS
    numberOfPushes64++;
    #endif

    return ;
  }

  void queuePop64(MARC::ThreadSafeQueue<int64_t> *queue, int64_t *val) { 
    queue->waitPop(*val); 

    return ;
  }


  /**********************************************************************
   *                DOALL
   **********************************************************************/
  static void * NOELLE_DOALLTrampoline (void *args){
    #ifdef RUNTIME_PROFILE
    auto clocks_start = rdtsc_s();
    #endif

    /*
     * Fetch the arguments.
     */
    auto DOALLArgs = (DOALL_args_t *) args;

    /*
     * Invoke
     */
    DOALLArgs->parallelizedLoop(DOALLArgs->env, DOALLArgs->coreID, DOALLArgs->numCores, DOALLArgs->chunkSize);
    #ifdef RUNTIME_PROFILE
    auto clocks_end = rdtsc_e();
    clocks_starts[DOALLArgs->coreID] = clocks_start;
    clocks_ends[DOALLArgs->coreID] = clocks_end;
    #endif

    return NULL;
  }

  DispatcherInfo NOELLE_DOALLDispatcher (
    void (*parallelizedLoop)(void *, int64_t, int64_t, int64_t), 
    void *env, 
    int64_t maxNumberOfCores, 
    int64_t chunkSize
    ){
    #ifdef RUNTIME_PROFILE
    auto clocks_start = rdtsc_s();
    #endif

    /*
     * Set the number of cores to use.
     */
    auto numCores = runtime.reserveCores(maxNumberOfCores);
    #ifdef RUNTIME_PRINT
    std::cerr << "Starting dispatcher: num cores " << numCores << ", chunk size: " << chunkSize << std::endl;
    #endif

    /*
     * Allocate the memory to store the arguments.
     */
    uint32_t doallMemoryIndex;
    nk_virgil_task_t *taskIDs;
    auto argsForAllCores = runtime.getDOALLArgs(numCores, &doallMemoryIndex, &taskIDs);

    /*
     * Submit DOALL tasks.
     */
    for (auto i = 0; i < numCores; ++i) {

      /*
       * Prepare the arguments.
       */
      auto argsPerCore = &argsForAllCores[i];
      argsPerCore->parallelizedLoop = parallelizedLoop;
      argsPerCore->env = env;
      argsPerCore->numCores = numCores;
      argsPerCore->chunkSize = chunkSize;

      /*
       * Submit
       */
      taskIDs[i] = nk_virgil_submit_task_to_any_cpu(NOELLE_DOALLTrampoline, argsPerCore);
    }
    #ifdef RUNTIME_PROFILE
    auto clocks_after_fork = rdtsc_e();
    #endif

    /*
     * Wait for DOALL tasks.
     */
    #ifdef RUNTIME_PROFILE
    auto clocks_before_join = rdtsc_s();
    #endif
    for (auto i = 0; i < numCores; ++i) {
      void *outputMemory;
      nk_virgil_wait_for_task_completion(taskIDs[i], &outputMemory);
    }
    #ifdef RUNTIME_PROFILE
    auto clocks_after_join = rdtsc_e();
    auto clocks_before_cleanup = rdtsc_s();
    #endif

    /*
     * Free the cores and memory.
     */
    runtime.releaseCores(numCores);
    runtime.releaseDOALLArgs(doallMemoryIndex);

    /*
     * Prepare the return value.
     */
    DispatcherInfo dispatcherInfo;
    dispatcherInfo.numberOfThreadsUsed = numCores;
    #ifdef RUNTIME_PROFILE
    auto clocks_after_cleanup = rdtsc_s();
    nk_virgil_spinlock_lock(&printLock);
    std::cerr << "XAN: Start         = " << clocks_start << "\n";
    std::cerr << "XAN: Setup overhead         = " << clocks_after_fork - clocks_start << " clocks\n";
    std::cerr << "XAN: Start joining = " << clocks_after_fork << "\n";
    for (auto i=0; i < numCores; i++){
      std::cerr << "Thread " << i << ": Start = " << clocks_starts[i] << "\n";
      std::cerr << "Thread " << i << ": End   = " << clocks_ends[i] << "\n";
      std::cerr << "Thread " << i << ": Delta = " << clocks_ends[i] - clocks_starts[i] << "\n";
    }
    std::cerr << "XAN: Joined        = " << clocks_after_join << "\n";
    std::cerr << "XAN: Joining delta = " << clocks_after_join - clocks_before_join << "\n";

    uint64_t start_min = 0;
    uint64_t start_max = 0;
    for (auto i=0; i < numCores; i++){
      if (  false
            || (start_min == 0)
            || (clocks_starts[i] < start_min)
        ){
        start_min = clocks_starts[i];
      }
      if (clocks_starts[i] > start_max){
        start_max = clocks_starts[i];
      }
    }
    std::cerr << "XAN: Thread starts min = " << start_min << "\n";
    std::cerr << "XAN: Thread starts max = " << start_max << "\n";
    std::cerr << "XAN: Task starting overhead = " << start_max - start_min << "\n";

    uint64_t end_max = 0;
    uint64_t lastThreadID = 0;
    for (auto i=0; i < numCores; i++){
      if (clocks_ends[i] > end_max){
        lastThreadID = i;
        end_max = clocks_ends[i];
      }
    }
    std::cerr << "XAN: Last thread ended = " << end_max << " (thread " << lastThreadID << ")\n";
    std::cerr << "XAN: Joining overhead       = " << clocks_after_join - end_max << "\n";

    nk_virgil_spinlock_unlock(&printLock);
    #endif

    return dispatcherInfo;
  }

  #ifdef RUNTIME_PRINT
  void *mySSGlobal = nullptr;
  #endif


  /**********************************************************************
   *                HELIX
   **********************************************************************/
  typedef struct {
    void (*parallelizedLoop)(void *, void *, void *, void *, int64_t, int64_t, uint64_t *);
    void *env ;
    void *loopCarriedArray;
    void *ssArrayPast;
    void *ssArrayFuture;
    uint64_t coreID;
    uint64_t numCores;
    uint64_t *loopIsOverFlag;
  } NOELLE_HELIX_args_t ;

  static void * NOELLE_HELIXTrampoline (void *args){

    /*
     * Fetch the arguments.
     */
    auto HELIX_args = (NOELLE_HELIX_args_t *) args;

    /*
     * Invoke
     */
    HELIX_args->parallelizedLoop(
      HELIX_args->env, 
      HELIX_args->loopCarriedArray, 
      HELIX_args->ssArrayPast, 
      HELIX_args->ssArrayFuture, 
      HELIX_args->coreID,
      HELIX_args->numCores,
      HELIX_args->loopIsOverFlag
      );

    return NULL;
  }

  static void HELIX_helperThread (void *ssArray, uint32_t numOfsequentialSegments, uint64_t *theLoopIsOver){

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

  static DispatcherInfo NOELLE_HELIX_dispatcher (
    void (*parallelizedLoop)(void *, void *, void *, void *, int64_t, int64_t, uint64_t *), 
    void *env,
    void *loopCarriedArray,
    int64_t maxNumberOfCores, 
    int64_t numOfsequentialSegments,
    bool LIO
    ){

    /*
     * Reserve the cores.
     */
    auto numCores = runtime.reserveCores(maxNumberOfCores);

    /*
     * Allocate the sequential segment arrays.
     * We need numCores - 1 arrays.
     */
    auto numOfSSArrays = numCores;
    if (!LIO){
      numOfSSArrays = 1;
    }
    void *ssArrays = NULL;
    auto ssSize = CACHE_LINE_SIZE;
    auto ssArraySize = ssSize * numOfsequentialSegments;
    if (numOfsequentialSegments > 0){

      /*
       * Allocate the sequential segment arrays.
       */
      posix_memalign(&ssArrays, CACHE_LINE_SIZE, ssArraySize * numOfSSArrays);
      if (ssArrays == NULL){
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
          auto lock = (nk_virgil_spinlock_t *)(((uint64_t)ssArray) + (lockID * ssSize));

          /*
           * Initialize the lock.
           */
          nk_virgil_spinlock_init(lock);

          /*
           * If the sequential segment is not for core 0, then we need to lock it.
           */
          if (i > 0){
            nk_virgil_spinlock_lock(lock);
          }
        }
      }
    }

    /*
     * Allocate the arguments for the cores.
     */
    NOELLE_HELIX_args_t *argsForAllCores;
    posix_memalign((void **)&argsForAllCores, CACHE_LINE_SIZE, sizeof(NOELLE_HELIX_args_t) * numCores);

    /*
     * Launch threads
     */
    uint64_t loopIsOverFlag = 0;
    auto localFutures = (nk_virgil_task_t *) malloc(sizeof(nk_virgil_task_t) * numCores);
    for (auto i = 0; i < numCores; ++i) {

      /*
       * Identify the past and future sequential segment arrays.
       */
      auto pastID = i % numOfSSArrays;
      auto futureID = (i + 1) % numOfSSArrays;

      /*
       * Fetch the sequential segment array for the current thread.
       */
      auto ssArrayPast = (void *)(((uint64_t)ssArrays) + (pastID * ssArraySize));
      auto ssArrayFuture = (void *)(((uint64_t)ssArrays) + (futureID * ssArraySize));
      #ifdef RUNTIME_PRINT
      fprintf(stderr, "HelixDispatcher: defined ss past and future arrays: %ld %ld\n", (int *)ssArrayPast - (int *)mySSGlobal, (int *)ssArrayFuture - (int *)mySSGlobal);
      #endif

      /*
       * Prepare the arguments.
       */
      auto argsPerCore = &argsForAllCores[i];
      argsPerCore->parallelizedLoop = parallelizedLoop;
      argsPerCore->env = env;
      argsPerCore->loopCarriedArray = loopCarriedArray;
      argsPerCore->ssArrayPast = ssArrayPast;
      argsPerCore->ssArrayFuture = ssArrayFuture;
      argsPerCore->coreID = i;
      argsPerCore->numCores = numCores;
      argsPerCore->loopIsOverFlag = &loopIsOverFlag;

      /*
       * Set the affinity for both the thread and its helper.
       */
      //cpu_set_t cores;
      /*CPU_ZERO(&cores);
      auto physicalCore = i * 2;
      CPU_SET(physicalCore, &cores);
      CPU_SET(physicalCore + 1, &cores);
      */

      /*
       * Launch the thread.
       */
      localFutures[i] = nk_virgil_submit_task_to_specific_cpu(NOELLE_HELIXTrampoline, argsPerCore, i);

      /*
       * Launch the helper thread.
       */
      continue ;
      /*localFutures.push_back(pool.submitToCores(
        cores,
        HELIX_helperThread, 
        ssArrayPast,
        numOfsequentialSegments,
        &loopIsOverFlag
      ));*/
    }

    /*
     * Wait for the threads to end
     */
    for (auto i = 0; i < numCores; ++i) {
      void *outputMemory;
      nk_virgil_wait_for_task_completion(localFutures[i], &outputMemory);
    }

    /*
     * Free the cores and memory.
     */
    runtime.releaseCores(numCores);

    /*
     * Free the memory.
     */
    free(argsForAllCores);
    free(ssArrays);
    free(argsForAllCores);

    DispatcherInfo dispatcherInfo;
    dispatcherInfo.numberOfThreadsUsed = numCores;
    return dispatcherInfo;
  }

  DispatcherInfo NOELLE_HELIX_dispatcher_sequentialSegments (
    void (*parallelizedLoop)(void *, void *, void *, void *, int64_t, int64_t, uint64_t *), 
    void *env,
    void *loopCarriedArray,
    int64_t numCores, 
    int64_t numOfsequentialSegments
    ){
    return NOELLE_HELIX_dispatcher(parallelizedLoop, env, loopCarriedArray, numCores, numOfsequentialSegments, true);
  }

  DispatcherInfo NOELLE_HELIX_dispatcher_criticalSections (
    void (*parallelizedLoop)(void *, void *, void *, void *, int64_t, int64_t, uint64_t *), 
    void *env,
    void *loopCarriedArray,
    int64_t numCores, 
    int64_t numOfsequentialSegments
    ){
    return NOELLE_HELIX_dispatcher(parallelizedLoop, env, loopCarriedArray, numCores, numOfsequentialSegments, false);
  }

  void HELIX_wait (
    void *sequentialSegment
    ){

    /*
     * Fetch the spinlock
     */
    auto ss = (nk_virgil_spinlock_t *) sequentialSegment;

    /*
     * Wait
     */
    nk_virgil_spinlock_lock(ss);

    return ;
  }

  void HELIX_signal (
    void *sequentialSegment
    ){

    /*
     * Fetch the spinlock
     */
    auto ss = (nk_virgil_spinlock_t *) sequentialSegment;

    /*
     * Signal
     */
    nk_virgil_spinlock_unlock(ss);

    return ;
  }


  /**********************************************************************
   *                DSWP
   **********************************************************************/
  typedef struct {
    stageFunctionPtr_t funcToInvoke;
    void *env;
    void *localQueues;
  } NOELLE_DSWP_args_t ;

  void stageExecuter(void (*stage)(void *, void *), void *env, void *queues){ 
    return stage(env, queues);
  }

  static void * NOELLE_DSWPTrampoline (void *args){

    /*
     * Fetch the arguments.
     */
    auto DSWPArgs = (NOELLE_DSWP_args_t *) args;

    /*
     * Invoke
     */
    DSWPArgs->funcToInvoke(DSWPArgs->env, DSWPArgs->localQueues);

    return NULL;
  }

  DispatcherInfo NOELLE_DSWPDispatcher (
    void *env, 
    int64_t *queueSizes, 
    void *stages, 
    int64_t numberOfStages, 
    int64_t numberOfQueues
    ){
    #ifdef RUNTIME_PRINT
    std::cerr << "Starting dispatcher: num stages " << numberOfStages << ", num queues: " << numberOfQueues << std::endl;
    #endif

    /*
     * Reserve the cores.
     */
    auto numCores = runtime.reserveCores(numberOfStages);

    /*
     * Allocate the communication queues.
     */
    void *localQueues[numberOfQueues];
    for (auto i = 0; i < numberOfQueues; ++i) {
      switch (queueSizes[i]) {
        case 1:
          localQueues[i] = new MARC::ThreadSafeNautilusQueue<int8_t>();
          break;
        case 8:
          localQueues[i] = new MARC::ThreadSafeNautilusQueue<int8_t>();
          break;
        case 16:
          localQueues[i] = new MARC::ThreadSafeNautilusQueue<int16_t>();
          break;
        case 32:
          localQueues[i] = new MARC::ThreadSafeNautilusQueue<int32_t>();
          break;
        case 64:
          localQueues[i] = new MARC::ThreadSafeNautilusQueue<int64_t>();
          break;
        default:
          abort();
          break;
      }
    }
    #ifdef RUNTIME_PRINT
    std::cerr << "Made queues" << std::endl;
    #endif

    /*
     * Allocate the memory to store the arguments.
     */
    auto argsForAllCores = (NOELLE_DSWP_args_t *) malloc(sizeof(NOELLE_DSWP_args_t) * numberOfStages);

    /*
     * Submit DSWP tasks
     */
    auto localFutures = (nk_virgil_task_t *) malloc(sizeof(nk_virgil_task_t) * numberOfStages);
    auto allStages = (void **)stages;
    for (auto i = 0; i < numberOfStages; ++i) {

      /*
       * Prepare the arguments.
       */
      auto argsPerCore = &argsForAllCores[i];
      argsPerCore->funcToInvoke = reinterpret_cast<stageFunctionPtr_t>(reinterpret_cast<long long>(allStages[i]));
      argsPerCore->env = env;
      argsPerCore->localQueues = (void *) localQueues;

      /*
       * Submit
       */
      localFutures[i] = nk_virgil_submit_task_to_specific_cpu(NOELLE_DSWPTrampoline, argsPerCore, i);
    }

    /*
     * Wait for the tasks to complete.
     */
    for (auto i = 0; i < numberOfStages; ++i) {
      void *outputMemory;
      nk_virgil_wait_for_task_completion(localFutures[i], &outputMemory);
    }

    /*
     * Free the cores and memory.
     */
    runtime.releaseCores(numberOfStages);
    for (int i = 0; i < numberOfQueues; ++i) {
      switch (queueSizes[i]) {
        case 1:
          delete (MARC::ThreadSafeNautilusQueue<int8_t> *)(localQueues[i]);
          break;
        case 8:
          delete (MARC::ThreadSafeNautilusQueue<int8_t> *)(localQueues[i]);
          break;
        case 16:
          delete (MARC::ThreadSafeNautilusQueue<int16_t> *)(localQueues[i]);
          break;
        case 32:
          delete (MARC::ThreadSafeNautilusQueue<int32_t> *)(localQueues[i]);
          break;
        case 64:
          delete (MARC::ThreadSafeNautilusQueue<int64_t> *)(localQueues[i]);
          break;
        default:
          abort();
      }
    }
    free(argsForAllCores);

    #ifdef DSWP_STATS
    std::cout << "DSWP: 1 Byte pushes = " << numberOfPushes8 << std::endl;
    std::cout << "DSWP: 2 Bytes pushes = " << numberOfPushes16 << std::endl;
    std::cout << "DSWP: 4 Bytes pushes = " << numberOfPushes32 << std::endl;
    std::cout << "DSWP: 8 Bytes pushes = " << numberOfPushes64 << std::endl;
    #endif

    DispatcherInfo dispatcherInfo;
    dispatcherInfo.numberOfThreadsUsed = numberOfStages;
    return dispatcherInfo;
  }

}

NoelleRuntime::NoelleRuntime()
  : 
    doallMemoryChunks{0}
  , doallMemoryTasks{nullptr}
  {

  /*
   * Set the number of cores in the system.
   */
  this->maxCores = this->getMaximumNumberOfCores();
  this->NOELLE_idleCores = maxCores;

  nk_virgil_spinlock_init(&this->spinLock);
  nk_virgil_spinlock_init(&this->doallMemoryLock);
  #ifdef RUNTIME_PROFILE
  nk_virgil_spinlock_init(&printLock);
  #endif

  return ;
}

DOALL_args_t * NoelleRuntime::getDOALLArgs (uint32_t cores, uint32_t *index, nk_virgil_task_t **taskIDs){
  DOALL_args_t *argsForAllCores = nullptr;

  /*
   * Check if we can reuse a previously-allocated memory region.
   */
  nk_virgil_spinlock_lock(&this->doallMemoryLock);
  auto doallMemoryNumberOfChunks = this->doallMemoryAvailability.size();
  for (auto i=0; i < doallMemoryNumberOfChunks; i++){
    auto currentSize = this->doallMemorySizes[i];
    if (  true
          && (this->doallMemoryAvailability[i])
          && (currentSize >= cores)
       ){

      /*
       * Found a memory block that can be reused.
       */
      argsForAllCores = this->doallMemory[i];

      /*
       * Set the block as in use.
       */
      this->doallMemoryAvailability[i] = false;
      (*index) = i;
      (*taskIDs) = this->doallMemoryTasks[i];
      nk_virgil_spinlock_unlock(&this->doallMemoryLock);

      return argsForAllCores;
    }
  }

  /*
   * We couldn't find anything available.
   *
   * Allocate a new memory region.
   *
   * Step1: increase the number of chunks
   */
  this->doallMemoryChunks++;

  /*
   * Step2: allocate the memory used to store task's IDs.
   */
  if (this->doallMemoryTasks == nullptr){
    this->doallMemoryTasks = (nk_virgil_task_t **)malloc(sizeof(nk_virgil_task_t *) * this->doallMemoryChunks);
  } else {
    this->doallMemoryTasks = (nk_virgil_task_t **)realloc(this->doallMemoryTasks, sizeof(nk_virgil_task_t *) * this->doallMemoryChunks);
  }
  this->doallMemoryTasks[doallMemoryNumberOfChunks] = (nk_virgil_task_t *) malloc(sizeof(nk_virgil_task_t) * this->maxCores);
  (*taskIDs) = this->doallMemoryTasks[doallMemoryNumberOfChunks];

  /*
   * Step3: allocate the memory about tagging whether a chunk is available or not.
   */
  this->doallMemoryAvailability.push_back(false);

  /*
   * Step4: allocate the memory to use for DOALL.
   */
  this->doallMemorySizes.push_back(cores);
  posix_memalign((void **)&argsForAllCores, CACHE_LINE_SIZE, sizeof(DOALL_args_t) * cores);
  this->doallMemory.push_back(argsForAllCores);

  nk_virgil_spinlock_unlock(&this->doallMemoryLock);

  /*
   * Set the index.
   */
  (*index) = doallMemoryNumberOfChunks;

  /*
   * Initialize the memory.
   */ 
  for (auto i = 0; i < cores; ++i) {
    auto argsPerCore = &argsForAllCores[i];
    argsPerCore->coreID = i;
  }

  return argsForAllCores;
}

void NoelleRuntime::releaseDOALLArgs (uint32_t index){
  nk_virgil_spinlock_lock(&this->doallMemoryLock);
  this->doallMemoryAvailability[index] = true;
  nk_virgil_spinlock_unlock(&this->doallMemoryLock);
  return ;
}

uint32_t NoelleRuntime::reserveCores (uint32_t coresRequested){
 
  /*
   * Reserve the number of cores available.
   */
  nk_virgil_spinlock_lock(&this->spinLock);
  auto numCores = this->NOELLE_idleCores > coresRequested ? coresRequested : NOELLE_idleCores;
  if (numCores < 1){
    numCores = 1;
  }
  this->NOELLE_idleCores -= numCores;
  nk_virgil_spinlock_unlock(&this->spinLock);

  return numCores;
}
    
void NoelleRuntime::releaseCores (uint32_t coresReleased){
  nk_virgil_spinlock_lock(&this->spinLock);
  this->NOELLE_idleCores += coresReleased;
  nk_virgil_spinlock_unlock(&this->spinLock);

  return ;
}

uint32_t NoelleRuntime::getMaximumNumberOfCores (void){
  static int cores = 0;

  /*
   * Check if we have already computed the number of cores.
   */
  if (cores == 0){

    /*
     * Compute the number of cores.
     */
    cores = nk_virgil_get_num_cpus();
  }

  return cores;
}
