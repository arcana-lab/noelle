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
#include <assert.h>

#include <ThreadSafeQueue.hpp>
#include <ThreadSafeLockFreeQueue.hpp>
#include <ThreadPools.hpp>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>
#include <iostream>

/*
 * OPTIONS
 */
//#define RUNTIME_PROFILE
//#define RUNTIME_PRINT
//#define DSWP_STATS

using namespace arcana::virgil;

#define CACHE_LINE_SIZE 64

#ifdef DSWP_STATS
static int64_t numberOfPushes8 = 0;
static int64_t numberOfPushes16 = 0;
static int64_t numberOfPushes32 = 0;
static int64_t numberOfPushes64 = 0;
#endif

typedef struct {
  void (*parallelizedLoop)(void *, int64_t, int64_t, int64_t);
  void *env;
  int64_t coreID;
  int64_t numCores;
  int64_t chunkSize;
  pthread_spinlock_t endLock;
} DOALL_args_t;

class NoelleRuntime {
public:
  NoelleRuntime();

  uint32_t reserveCores(uint32_t coresRequested);

  void releaseCores(uint32_t coresReleased);

  uint32_t getAvailableCores(void);

  DOALL_args_t *getDOALLArgs(uint32_t cores, uint32_t *index);

  void releaseDOALLArgs(uint32_t index);

  ThreadPoolForCSingleQueue *virgil;

  ~NoelleRuntime(void);

private:
  mutable pthread_spinlock_t doallMemoryLock;
  std::vector<uint32_t> doallMemorySizes;
  std::vector<bool> doallMemoryAvailability;
  std::vector<DOALL_args_t *> doallMemory;

  uint32_t getMaximumNumberOfCores(void);

  /*
   * Current number of idle cores.
   */
  int32_t NOELLE_idleCores;

  /*
   * Maximum number of cores.
   */
  uint32_t maxCores;

  mutable pthread_spinlock_t spinLock;
};

#ifdef RUNTIME_PROFILE
uint64_t clocks_starts[64];
uint64_t clocks_ends[64];
uint64_t clocks_dispatch_starts[64];
uint64_t clocks_dispatch_ends[64];
#endif

/*
 * Synchronize printing
 */
#ifdef RUNTIME_PROFILE
#  define RUNTIME_PRINT_LOCK
#endif
#ifdef RUNTIME_PRINT
#  define RUNTIME_PRINT_LOCK
#endif
#ifdef RUNTIME_PRINT_LOCK
pthread_spinlock_t printLock;
#endif

static NoelleRuntime runtime{};

extern "C" {

/************************************ NOELLE public APIs **************/
class DispatcherInfo {
public:
  int32_t numberOfThreadsUsed;
  int64_t unusedVariableToPreventOptIfStructHasOnlyOneVariable;
};

/*
 * Dispatch tasks to run a DOALL loop.
 */
DispatcherInfo NOELLE_DOALLDispatcher(
    void (*parallelizedLoop)(void *, int64_t, int64_t, int64_t),
    void *env,
    int64_t maxNumberOfCores,
    int64_t chunkSize);

/*
 * Dispatch tasks to run a HELIX loop.
 */
DispatcherInfo NOELLE_HELIX_dispatcher_sequentialSegments(
    void (*parallelizedLoop)(void *,
                             void *,
                             void *,
                             void *,
                             int64_t,
                             int64_t,
                             uint64_t *),
    void *env,
    void *loopCarriedArray,
    int64_t numCores,
    int64_t numOfsequentialSegments);

DispatcherInfo NOELLE_HELIX_dispatcher_criticalSections(
    void (*parallelizedLoop)(void *,
                             void *,
                             void *,
                             void *,
                             int64_t,
                             int64_t,
                             uint64_t *),
    void *env,
    void *loopCarriedArray,
    int64_t numCores,
    int64_t numOfsequentialSegments);

DispatcherInfo NOELLE_DSWPDispatcher(void *env,
                                     int64_t *queueSizes,
                                     void *stages,
                                     int64_t numberOfStages,
                                     int64_t numberOfQueues);

/******************************************* Utils ********************/
#ifdef RUNTIME_PROFILE
static __inline__ int64_t rdtsc_s(void) {
  unsigned a, d;
  asm volatile("cpuid" ::: "%rax", "%rbx", "%rcx", "%rdx");
  asm volatile("rdtsc" : "=a"(a), "=d"(d));
  return ((unsigned long)a) | (((unsigned long)d) << 32);
}
static __inline__ int64_t rdtsc_e(void) {
  unsigned a, d;
  asm volatile("rdtscp" : "=a"(a), "=d"(d));
  asm volatile("cpuid" ::: "%rax", "%rbx", "%rcx", "%rdx");
  return ((unsigned long)a) | (((unsigned long)d) << 32);
}
#endif

/************************************* NOELLE API implementations ***/
typedef void (*stageFunctionPtr_t)(void *, void *);

void printReachedS(std::string s) {
  auto outS = "Reached: " + s;
  printf("%s\n", outS.c_str());
}

void printReachedI(int i) {
  printf("Reached: %d\n", i);
}

void printPushedP(int32_t *p) {
  printf("Pushed: %p\n", p);
}

void printPulledP(int32_t *p) {
  printf("Pulled: %p\n", p);
}

void queuePush8(ThreadSafeQueue<int8_t> *queue, int8_t *val) {
  queue->push(*val);

#ifdef DSWP_STATS
  numberOfPushes8++;
#endif

  return;
}

void queuePop8(ThreadSafeQueue<int8_t> *queue, int8_t *val) {
  queue->waitPop(*val);
  return;
}

void queuePush16(ThreadSafeQueue<int16_t> *queue, int16_t *val) {
  queue->push(*val);

#ifdef DSWP_STATS
  numberOfPushes16++;
#endif

  return;
}

void queuePop16(ThreadSafeQueue<int16_t> *queue, int16_t *val) {
  queue->waitPop(*val);
}

void queuePush32(ThreadSafeQueue<int32_t> *queue, int32_t *val) {
  queue->push(*val);

#ifdef DSWP_STATS
  numberOfPushes32++;
#endif

  return;
}

void queuePop32(ThreadSafeQueue<int32_t> *queue, int32_t *val) {
  queue->waitPop(*val);
}

void queuePush64(ThreadSafeQueue<int64_t> *queue, int64_t *val) {
  queue->push(*val);

#ifdef DSWP_STATS
  numberOfPushes64++;
#endif

  return;
}

void queuePop64(ThreadSafeQueue<int64_t> *queue, int64_t *val) {
  queue->waitPop(*val);

  return;
}

/**********************************************************************
 *                DOALL
 **********************************************************************/
static void NOELLE_DOALLTrampoline(void *args) {
#ifdef RUNTIME_PROFILE
  auto clocks_start = rdtsc_s();
#endif

  /*
   * Fetch the arguments.
   */
  auto DOALLArgs = (DOALL_args_t *)args;

  /*
   * Invoke
   */
  DOALLArgs->parallelizedLoop(DOALLArgs->env,
                              DOALLArgs->coreID,
                              DOALLArgs->numCores,
                              DOALLArgs->chunkSize);
#ifdef RUNTIME_PROFILE
  auto clocks_end = rdtsc_e();
  clocks_starts[DOALLArgs->coreID] = clocks_start;
  clocks_ends[DOALLArgs->coreID] = clocks_end;
#endif

  pthread_spin_unlock(&(DOALLArgs->endLock));
  return;
}

DispatcherInfo NOELLE_DOALLDispatcher(
    void (*parallelizedLoop)(void *, int64_t, int64_t, int64_t),
    void *env,
    int64_t maxNumberOfCores,
    int64_t chunkSize) {
#ifdef RUNTIME_PROFILE
  auto clocks_start = rdtsc_s();
#endif

  /*
   * Fetch VIRGIL
   */
  auto virgil = runtime.virgil;

  /*
   * Set the number of cores to use.
   */
  auto numCores = runtime.reserveCores(maxNumberOfCores);
#ifdef RUNTIME_PRINT
  std::cerr << "DOALL: Dispatcher: Start" << std::endl;
  std::cerr << "DOALL: Dispatcher:   Number of cores: " << numCores
            << std::endl;
  std::cerr << "DOALL: Dispatcher:   Chunk size: " << chunkSize << std::endl;
#endif

  /*
   * Allocate the memory to store the arguments.
   */
  uint32_t doallMemoryIndex;
  auto argsForAllCores = runtime.getDOALLArgs(numCores - 1, &doallMemoryIndex);

  /*
   * Submit DOALL tasks.
   */
  for (auto i = 0; i < (numCores - 1); ++i) {

    /*
     * Prepare the arguments.
     */
    auto argsPerCore = &argsForAllCores[i];
    argsPerCore->parallelizedLoop = parallelizedLoop;
    argsPerCore->env = env;
    argsPerCore->numCores = numCores;
    argsPerCore->chunkSize = chunkSize;

#ifdef RUNTIME_PROFILE
    clocks_dispatch_starts[i] = rdtsc_s();
#endif

    /*
     * Submit
     */
    virgil->submitAndDetach(NOELLE_DOALLTrampoline, argsPerCore);

#ifdef RUNTIME_PROFILE
    clocks_dispatch_ends[i] = rdtsc_s();
#endif
  }
#ifdef RUNTIME_PRINT
  std::cerr << "DOALL: Dispatcher:   Submitted " << numCores
            << " task instances" << std::endl;
#endif
#ifdef RUNTIME_PROFILE
  auto clocks_after_fork = rdtsc_e();
#endif

  /*
   * Run a task.
   */
  parallelizedLoop(env, numCores - 1, numCores, chunkSize);

/*
 * Wait for the remaining DOALL tasks.
 */
#ifdef RUNTIME_PROFILE
  auto clocks_before_join = rdtsc_s();
#endif
  for (auto i = 0; i < (numCores - 1); ++i) {
    pthread_spin_lock(&(argsForAllCores[i].endLock));
  }
#ifdef RUNTIME_PRINT
  std::cerr << "DOALL: Dispatcher:   All task instances have completed"
            << std::endl;
#endif
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
  pthread_spin_lock(&printLock);
  std::cerr << "XAN: Start         = " << clocks_start << "\n";
  std::cerr
      << "XAN: Setup overhead         = " << clocks_after_fork - clocks_start
      << " clocks\n";
  uint64_t totalDispatch = 0;
  for (auto i = 0; i < (numCores - 1); i++) {
    totalDispatch += (clocks_dispatch_ends[i] - clocks_dispatch_starts[i]);
  }
  std::cerr << "XAN:    Dispatch overhead         = " << totalDispatch
            << " clocks\n";
  std::cerr << "XAN: Start joining = " << clocks_after_fork << "\n";
  for (auto i = 0; i < (numCores - 1); i++) {
    std::cerr << "Thread " << i << ": Start = " << clocks_starts[i] << "\n";
    std::cerr << "Thread " << i << ": End   = " << clocks_ends[i] << "\n";
    std::cerr << "Thread " << i
              << ": Delta = " << clocks_ends[i] - clocks_starts[i] << "\n";
  }
  std::cerr << "XAN: Joined        = " << clocks_after_join << "\n";
  std::cerr << "XAN: Joining delta = " << clocks_after_join - clocks_before_join
            << "\n";

  uint64_t start_min = 0;
  uint64_t start_max = 0;
  for (auto i = 0; i < (numCores - 1); i++) {
    if (false || (start_min == 0) || (clocks_starts[i] < start_min)) {
      start_min = clocks_starts[i];
    }
    if (clocks_starts[i] > start_max) {
      start_max = clocks_starts[i];
    }
  }
  std::cerr << "XAN: Thread starts min = " << start_min << "\n";
  std::cerr << "XAN: Thread starts max = " << start_max << "\n";
  std::cerr << "XAN: Task starting overhead = " << start_max - start_min
            << "\n";

  uint64_t end_max = 0;
  uint64_t lastThreadID = 0;
  for (auto i = 0; i < (numCores - 1); i++) {
    if (clocks_ends[i] > end_max) {
      lastThreadID = i;
      end_max = clocks_ends[i];
    }
  }
  std::cerr << "XAN: Last thread ended = " << end_max << " (thread "
            << lastThreadID << ")\n";
  std::cerr << "XAN: Joining overhead       = " << clocks_after_join - end_max
            << "\n";

  pthread_spin_unlock(&printLock);
#endif
#ifdef RUNTIME_PRINT
  std::cerr << "DOALL: Dispatcher: Exit" << std::endl;
#endif

  return dispatcherInfo;
}

/**********************************************************************
 *                HELIX
 **********************************************************************/
typedef struct {
  void (*parallelizedLoop)(void *,
                           void *,
                           void *,
                           void *,
                           int64_t,
                           int64_t,
                           uint64_t *);
  void *env;
  void *loopCarriedArray;
  void *ssArrayPast;
  void *ssArrayFuture;
  uint64_t coreID;
  uint64_t numCores;
  uint64_t *loopIsOverFlag;
  pthread_spinlock_t endLock;
} NOELLE_HELIX_args_t;

static void NOELLE_HELIXTrampoline(void *args) {

  /*
   * Fetch the arguments.
   */
  auto HELIX_args = (NOELLE_HELIX_args_t *)args;

  /*
   * Invoke
   */
  HELIX_args->parallelizedLoop(HELIX_args->env,
                               HELIX_args->loopCarriedArray,
                               HELIX_args->ssArrayPast,
                               HELIX_args->ssArrayFuture,
                               HELIX_args->coreID,
                               HELIX_args->numCores,
                               HELIX_args->loopIsOverFlag);

  pthread_spin_unlock(&(HELIX_args->endLock));
  return;
}

static void HELIX_helperThread(void *ssArray,
                               uint32_t numOfsequentialSegments,
                               uint64_t *theLoopIsOver) {

  while ((*theLoopIsOver) == 0) {

    /*
     * Prefetch all sequential segment cache lines of the current loop
     * iteration.
     */
    for (auto i = 0; ((*theLoopIsOver) == 0) && (i < numOfsequentialSegments);
         i++) {

      /*
       * Fetch the pointer.
       */
      auto ptr = (uint64_t *)(((uint64_t)ssArray) + (i * CACHE_LINE_SIZE));

      /*
       * Prefetch the cache line for the current sequential segment.
       */
      while (((*theLoopIsOver) == 0) && ((*ptr) == 0))
        ;
    }
  }

  return;
}

static DispatcherInfo NOELLE_HELIX_dispatcher(
    void (*parallelizedLoop)(void *,
                             void *,
                             void *,
                             void *,
                             int64_t,
                             int64_t,
                             uint64_t *),
    void *env,
    void *loopCarriedArray,
    int64_t maxNumberOfCores,
    int64_t numOfsequentialSegments,
    bool LIO) {

  /*
   * Assumptions.
   */
  assert(parallelizedLoop != NULL);
  assert(env != NULL);
  assert(maxNumberOfCores > 1);

  /*
   * Fetch VIRGIL
   */
  auto virgil = runtime.virgil;

  /*
   * Reserve the cores.
   */
  auto numCores = runtime.reserveCores(maxNumberOfCores);
  assert(numCores >= 1);

#ifdef RUNTIME_PRINT
  pthread_spin_lock(&printLock);
  std::cerr << "HELIX: dispatcher: Start" << std::endl;
  std::cerr << "HELIX: dispatcher:   Number of sequential segments = "
            << numOfsequentialSegments << std::endl;
  std::cerr << "HELIX: dispatcher:   Number of cores = " << numCores
            << std::endl;
  pthread_spin_unlock(&printLock);
#endif

  /*
   * Allocate the sequential segment arrays.
   * We need numCores - 1 arrays.
   */
  auto numOfSSArrays = numCores;
  if (!LIO) {
    numOfSSArrays = 1;
  }
  void *ssArrays = nullptr;
  auto ssSize = CACHE_LINE_SIZE;
  auto ssArraySize = ssSize * numOfsequentialSegments;
  if (numOfsequentialSegments > 0) {

    /*
     * Allocate the sequential segment arrays.
     */
    posix_memalign(&ssArrays, CACHE_LINE_SIZE, ssArraySize * numOfSSArrays);
    if (ssArrays == NULL) {
      std::cerr << "HELIX: dispatcher: ERROR = not enough memory to allocate "
                << numCores << " sequential segment arrays" << std::endl;
      abort();
    }

    /*
     * Initialize the sequential segment arrays.
     */
    for (auto i = 0; i < numOfSSArrays; i++) {

      /*
       * Fetch the current sequential segment array.
       */
      auto ssArray = (void *)(((uint64_t)ssArrays) + (i * ssArraySize));

      /*
       * Initialize the locks.
       */
      for (auto lockID = 0; lockID < numOfsequentialSegments; lockID++) {

        /*
         * Fetch the pointer to the current lock.
         */
        auto lock =
            (pthread_spinlock_t *)(((uint64_t)ssArray) + (lockID * ssSize));

        /*
         * Initialize the lock.
         */
        pthread_spin_init(lock, PTHREAD_PROCESS_PRIVATE);

        /*
         * If the sequential segment is not for core 0, then we need to lock it.
         */
        if (i > 0) {
          pthread_spin_lock(lock);
        }
      }
    }
  }

  /*
   * Allocate the arguments for the cores.
   */
  NOELLE_HELIX_args_t *argsForAllCores;
  posix_memalign((void **)&argsForAllCores,
                 CACHE_LINE_SIZE,
                 sizeof(NOELLE_HELIX_args_t) * (numCores - 1));

  /*
   * Launch threads
   */
  uint64_t loopIsOverFlag = 0;
  cpu_set_t cores;
  for (auto i = 0; i < (numCores - 1); ++i) {

    /*
     * Identify the past and future sequential segment arrays.
     */
    auto pastID = i % numOfSSArrays;
    auto futureID = (i + 1) % numOfSSArrays;

    /*
     * Fetch the sequential segment array for the current thread.
     */
    auto ssArrayPast = (void *)(((uint64_t)ssArrays) + (pastID * ssArraySize));
    auto ssArrayFuture =
        (void *)(((uint64_t)ssArrays) + (futureID * ssArraySize));
#ifdef RUNTIME_PRINT
    pthread_spin_lock(&printLock);
    auto pastDelta = (int *)ssArrayPast - (int *)ssArrays;
    auto futureDelta = (int *)ssArrayFuture - (int *)ssArrays;
    std::cerr << "HELIX: dispatcher:   Task instance " << i << std::endl;
    std::cerr << "HELIX: dispatcher:     SS arrays: " << ssArrays << std::endl;
    std::cerr << "HELIX: dispatcher:       SS past: " << ssArrayPast
              << std::endl;
    std::cerr << "HELIX: dispatcher:       SS future: " << ssArrayFuture
              << std::endl;
    std::cerr
        << "HELIX: dispatcher:       SS past and future arrays: " << pastDelta
        << " " << futureDelta << std::endl;
    pthread_spin_unlock(&printLock);
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
    pthread_spin_init(&(argsPerCore->endLock), PTHREAD_PROCESS_PRIVATE);
    pthread_spin_lock(&(argsPerCore->endLock));

    /*
     * Set the affinity for both the thread and its helper.
     */
    /*CPU_ZERO(&cores);
    auto physicalCore = i * 2;
    CPU_SET(physicalCore, &cores);
    CPU_SET(physicalCore + 1, &cores);
    */

    /*
     * Launch the thread.
     */
    virgil->submitAndDetach(NOELLE_HELIXTrampoline, argsPerCore);

    /*
     * Launch the helper thread.
     */
    continue;
    /*localFutures.push_back(pool.submitToCores(
      cores,
      HELIX_helperThread,
      ssArrayPast,
      numOfsequentialSegments,
      &loopIsOverFlag
    ));*/
  }
#ifdef RUNTIME_PRINT
  pthread_spin_lock(&printLock);
  std::cerr << "HELIX: dispatcher:   Submitted all task instances" << std::endl;
  pthread_spin_unlock(&printLock);
  int futureGotten = 0;
#endif

  /*
   * Run a task.
   */
  auto pastID = (numCores - 1) % numOfSSArrays;
  auto futureID = 0;
  auto ssArrayPast = (void *)(((uint64_t)ssArrays) + (pastID * ssArraySize));
  auto ssArrayFuture = ssArrays;
  parallelizedLoop(env,
                   loopCarriedArray,
                   ssArrayPast,
                   ssArrayFuture,
                   numCores - 1,
                   numCores,
                   &loopIsOverFlag);

  /*
   * Wait for the remaining HELIX tasks.
   */
  for (auto i = 0; i < (numCores - 1); ++i) {
    pthread_spin_lock(&(argsForAllCores[i].endLock));
  }
#ifdef RUNTIME_PRINT
  pthread_spin_lock(&printLock);
  std::cerr << "HELIX: dispatcher:   All task instances have completed"
            << std::endl;
  pthread_spin_unlock(&printLock);
#endif

  /*
   * Free the cores and memory.
   */
  runtime.releaseCores(numCores);

  /*
   * Free the memory.
   */
  free(argsForAllCores);
  free(ssArrays);

  /*
   * Exit
   */
#ifdef RUNTIME_PRINT
  pthread_spin_lock(&printLock);
  std::cerr << "HELIX: dispatcher: Exit" << std::endl;
  pthread_spin_unlock(&printLock);
#endif

  DispatcherInfo dispatcherInfo;
  dispatcherInfo.numberOfThreadsUsed = numCores;
  return dispatcherInfo;
}

DispatcherInfo NOELLE_HELIX_dispatcher_sequentialSegments(
    void (*parallelizedLoop)(void *,
                             void *,
                             void *,
                             void *,
                             int64_t,
                             int64_t,
                             uint64_t *),
    void *env,
    void *loopCarriedArray,
    int64_t numCores,
    int64_t numOfsequentialSegments) {
  return NOELLE_HELIX_dispatcher(parallelizedLoop,
                                 env,
                                 loopCarriedArray,
                                 numCores,
                                 numOfsequentialSegments,
                                 true);
}

DispatcherInfo NOELLE_HELIX_dispatcher_criticalSections(
    void (*parallelizedLoop)(void *,
                             void *,
                             void *,
                             void *,
                             int64_t,
                             int64_t,
                             uint64_t *),
    void *env,
    void *loopCarriedArray,
    int64_t numCores,
    int64_t numOfsequentialSegments) {
  return NOELLE_HELIX_dispatcher(parallelizedLoop,
                                 env,
                                 loopCarriedArray,
                                 numCores,
                                 numOfsequentialSegments,
                                 false);
}

void HELIX_wait(void *sequentialSegment) {

  /*
   * Fetch the spinlock
   */
  auto ss = (pthread_spinlock_t *)sequentialSegment;

#ifdef RUNTIME_PRINT
  assert(ss != NULL);
  pthread_spin_lock(&printLock);
  std::cerr << "HELIX: Waiting on sequential segment " << sequentialSegment
            << std::endl;
  pthread_spin_unlock(&printLock);
#endif

  /*
   * Wait
   */
  pthread_spin_lock(ss);

#ifdef RUNTIME_PRINT
  pthread_spin_lock(&printLock);
  std::cerr << "HELIX: Waited on sequential segment " << sequentialSegment
            << std::endl;
  pthread_spin_unlock(&printLock);
#endif

  return;
}

void HELIX_signal(void *sequentialSegment) {

  /*
   * Fetch the spinlock
   */
  auto ss = (pthread_spinlock_t *)sequentialSegment;

#ifdef RUNTIME_PRINT
  assert(ss != NULL);
  pthread_spin_lock(&printLock);
  std::cerr << "HELIX: Signaling on sequential segment " << sequentialSegment
            << std::endl;
  pthread_spin_unlock(&printLock);
#endif

  /*
   * Signal
   */
  pthread_spin_unlock(ss);

#ifdef RUNTIME_PRINT
  pthread_spin_lock(&printLock);
  std::cerr << "HELIX: Signaled on sequential segment " << sequentialSegment
            << std::endl;
  pthread_spin_unlock(&printLock);
#endif

  return;
}

/**********************************************************************
 *                DSWP
 **********************************************************************/
typedef struct {
  stageFunctionPtr_t funcToInvoke;
  void *env;
  void *localQueues;
  pthread_mutex_t endLock;
} NOELLE_DSWP_args_t;

void stageExecuter(void (*stage)(void *, void *), void *env, void *queues) {
  return stage(env, queues);
}

static void NOELLE_DSWPTrampoline(void *args) {

  /*
   * Fetch the arguments.
   */
  auto DSWPArgs = (NOELLE_DSWP_args_t *)args;

  /*
   * Invoke
   */
  DSWPArgs->funcToInvoke(DSWPArgs->env, DSWPArgs->localQueues);

  pthread_mutex_unlock(&(DSWPArgs->endLock));
  return;
}

DispatcherInfo NOELLE_DSWPDispatcher(void *env,
                                     int64_t *queueSizes,
                                     void *stages,
                                     int64_t numberOfStages,
                                     int64_t numberOfQueues) {
#ifdef RUNTIME_PRINT
  std::cerr << "Starting dispatcher: num stages " << numberOfStages
            << ", num queues: " << numberOfQueues << std::endl;
#endif

  /*
   * Fetch VIRGIL
   */
  auto virgil = runtime.virgil;

  /*
   * Reserve the cores.
   */
  auto numCores = runtime.reserveCores(numberOfStages);
  assert(numCores >= 1);

  /*
   * Allocate the communication queues.
   */
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
        std::cerr << "NOELLE: Runtime: QUEUE SIZE INCORRECT" << std::endl;
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
  auto argsForAllCores =
      (NOELLE_DSWP_args_t *)malloc(sizeof(NOELLE_DSWP_args_t) * numberOfStages);

  /*
   * Submit DSWP tasks
   */
  auto allStages = (void **)stages;
  for (auto i = 0; i < numberOfStages; ++i) {

    /*
     * Prepare the arguments.
     */
    auto argsPerCore = &argsForAllCores[i];
    argsPerCore->funcToInvoke = reinterpret_cast<stageFunctionPtr_t>(
        reinterpret_cast<long long>(allStages[i]));
    argsPerCore->env = env;
    argsPerCore->localQueues = (void *)localQueues;
    pthread_mutex_init(&(argsPerCore->endLock), NULL);
    pthread_mutex_lock(&(argsPerCore->endLock));

    /*
     * Submit
     */
    virgil->submitAndDetach(NOELLE_DSWPTrampoline, argsPerCore);
#ifdef RUNTIME_PRINT
    std::cerr << "Submitted stage" << std::endl;
#endif
  }
#ifdef RUNTIME_PRINT
  std::cerr << "Submitted pool" << std::endl;
#endif

  /*
   * Wait for the tasks to complete.
   */
  for (auto i = 0; i < numberOfStages; ++i) {
    pthread_mutex_lock(&(argsForAllCores[i].endLock));
  }
#ifdef RUNTIME_PRINT
  std::cerr << "Got all futures" << std::endl;
#endif

  /*
   * Free the cores and memory.
   */
  runtime.releaseCores(numCores);
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

uint32_t NOELLE_getAvailableCores(void) {
  auto idleCores = runtime.getAvailableCores();

  return idleCores;
}
}

NoelleRuntime::NoelleRuntime() {
  this->maxCores = this->getMaximumNumberOfCores();
  this->NOELLE_idleCores = maxCores;

  pthread_spin_init(&this->spinLock, 0);
  pthread_spin_init(&this->doallMemoryLock, 0);
#ifdef RUNTIME_PRINT_LOCK
  pthread_spin_init(&printLock, 0);
#endif

  /*
   * Allocate VIRGIL
   */
  this->virgil = new ThreadPoolForCSingleQueue(false, maxCores);

  return;
}

DOALL_args_t *NoelleRuntime::getDOALLArgs(uint32_t cores, uint32_t *index) {
  DOALL_args_t *argsForAllCores = nullptr;

  /*
   * Check if we can reuse a previously-allocated memory region.
   */
  pthread_spin_lock(&this->doallMemoryLock);
  auto doallMemoryNumberOfChunks = this->doallMemoryAvailability.size();
  for (auto i = 0; i < doallMemoryNumberOfChunks; i++) {
    auto currentSize = this->doallMemorySizes[i];
    if (true && (this->doallMemoryAvailability[i]) && (currentSize >= cores)) {

      /*
       * Found a memory block that can be reused.
       */
      argsForAllCores = this->doallMemory[i];

      /*
       * Set the block as in use.
       */
      this->doallMemoryAvailability[i] = false;
      (*index) = i;
      pthread_spin_unlock(&this->doallMemoryLock);

      return argsForAllCores;
    }
  }

  /*
   * We couldn't find anything available.
   *
   * Allocate a new memory region.
   */
  this->doallMemorySizes.push_back(cores);
  this->doallMemoryAvailability.push_back(false);
  posix_memalign((void **)&argsForAllCores,
                 CACHE_LINE_SIZE,
                 sizeof(DOALL_args_t) * cores);
  this->doallMemory.push_back(argsForAllCores);
  pthread_spin_unlock(&this->doallMemoryLock);

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
    pthread_spin_init(&(argsPerCore->endLock), 0);
    pthread_spin_lock(&(argsPerCore->endLock));
  }

  return argsForAllCores;
}

void NoelleRuntime::releaseDOALLArgs(uint32_t index) {
  pthread_spin_lock(&this->doallMemoryLock);
  this->doallMemoryAvailability[index] = true;
  pthread_spin_unlock(&this->doallMemoryLock);
  return;
}

uint32_t NoelleRuntime::reserveCores(uint32_t coresRequested) {

  /*
   * Reserve the number of cores available.
   */
  pthread_spin_lock(&this->spinLock);
  auto numCores = (this->NOELLE_idleCores >= coresRequested) ? coresRequested
                                                             : NOELLE_idleCores;
  if (numCores < 1) {
    numCores = 1;
  }
  this->NOELLE_idleCores -= numCores;
  pthread_spin_unlock(&this->spinLock);

  return numCores;
}

void NoelleRuntime::releaseCores(uint32_t coresReleased) {
  assert(coresReleased > 0);

  pthread_spin_lock(&this->spinLock);
  this->NOELLE_idleCores += coresReleased;
#ifdef DEBUG
  if (this->NOELLE_idleCores >= 0) {
    assert(this->NOELLE_idleCores <= ((uint32_t)this->maxCores));
  }
#endif
  pthread_spin_unlock(&this->spinLock);

  return;
}

uint32_t NoelleRuntime::getMaximumNumberOfCores(void) {
  static int cores = 0;

  /*
   * Check if we have already computed the number of cores.
   */
  if (cores == 0) {

    /*
     * Compute the number of cores.
     */
    auto envVar = getenv("NOELLE_CORES");
    if (envVar == nullptr) {
      cores = (std::thread::hardware_concurrency() / 2);
    } else {
      cores = atoi(envVar);
    }
  }

  return cores;
}

uint32_t NoelleRuntime::getAvailableCores(void) {

  /*
   * Get the number of cores available.
   */
  auto numCores = this->NOELLE_idleCores;
  if (numCores < 1) {
    numCores = 1;
  }

  return numCores;
}

NoelleRuntime::~NoelleRuntime(void) {
  delete this->virgil;
}
