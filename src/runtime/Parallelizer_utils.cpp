#include <stdlib.h>

#include <sched.h>

#include "rt/virgil/virgil.h"

extern "C" {

  /******************************************** NOELLE APIs ***********************************************/

  class DispatcherInfo {
    public:
      int32_t numberOfThreadsUsed;
      int64_t unusedVariableToPreventOptIfStructHasOnlyOneVariable;
  };

  /*
   * Return the number of cores to use for the parallelization.
   */
  int32_t NOELLE_getNumberOfCores (void);

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



  /**********************************************************************
   *                MISC
   **********************************************************************/
  int32_t NOELLE_getNumberOfCores (void){
    static int32_t cores = 0;

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

  typedef void (*stageFunctionPtr_t)(void *, void*);



  /**********************************************************************
   *                DOALL
   **********************************************************************/
  typedef struct {
    void (*parallelizedLoop)(void *, int64_t, int64_t, int64_t) ;
    void *env ;
    int64_t coreID ;
    int64_t numCores;
    int64_t chunkSize ;
  } DOALL_args_t ;

  void * NOELLE_DOALLTrampoline (void *args){

    /*
     * Fetch the arguments.
     */
    auto DOALLArgs = (DOALL_args_t *) args;

    /*
     * Invoke
     */
    DOALLArgs->parallelizedLoop(DOALLArgs->env, DOALLArgs->coreID, DOALLArgs->numCores, DOALLArgs->chunkSize);

    return NULL;
  }

  DispatcherInfo NOELLE_DOALLDispatcher (
    void (*parallelizedLoop)(void *, int64_t, int64_t, int64_t), 
    void *env, 
    int64_t maxNumberOfCores, 
    int64_t chunkSize
    ){

    /*
     * Set the number of cores to use.
     */
    auto runtimeNumberOfCores = NOELLE_getNumberOfCores();
    auto numCores = runtimeNumberOfCores > maxNumberOfCores ? maxNumberOfCores : runtimeNumberOfCores;

    /*
     * Allocate the memory to store the arguments.
     */
    DOALL_args_t *argsForAllCores;
    posix_memalign((void **)&argsForAllCores, CACHE_LINE_SIZE, sizeof(DOALL_args_t) * numCores);

    /*
     * Submit DOALL tasks.
     */
    auto localFutures = (nk_virgil_task_t *) malloc(sizeof(nk_virgil_task_t) * numCores);
    for (auto i = 0; i < numCores; ++i) {

      /*
       * Prepare the arguments.
       */
      auto argsPerCore = &argsForAllCores[i];
      argsPerCore->parallelizedLoop = parallelizedLoop;
      argsPerCore->env = env;
      argsPerCore->coreID = i;
      argsPerCore->numCores = numCores;
      argsPerCore->chunkSize = chunkSize;

      /*
       * Submit
       */
      localFutures[i] = nk_virgil_submit_task_to_any_cpu(NOELLE_DOALLTrampoline, argsPerCore);
    }

    /*
     * Wait for DOALL tasks.
     */
    for (auto i = 0; i < numCores; ++i) {
      void *outputMemory;
      nk_virgil_wait_for_task_completion(localFutures[i], &outputMemory);
    }

    /*
     * Free the memory.
     */
    free(argsForAllCores);

    DispatcherInfo dispatcherInfo;
    dispatcherInfo.numberOfThreadsUsed = numCores;
    return dispatcherInfo;
  }

}
