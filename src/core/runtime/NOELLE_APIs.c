#include <stdint.h>

typedef struct {
  int32_t numberOfThreadsUsed;
  int64_t unusedVariableToPreventOptIfStructHasOnlyOneVariable;
} DispatcherInfo;

extern DispatcherInfo NOELLE_DOALLDispatcher(
    void (*parallelizedLoop)(void *, int64_t, int64_t, int64_t),
    void *env,
    int64_t maxNumberOfCores,
    int64_t chunkSize);

extern void queuePush8(void *, int8_t *);
extern void queuePush16(void *, int16_t *);
extern void queuePush32(void *, int32_t *);
extern void queuePush64(void *, int64_t *);

extern void queuePop8(void *, int8_t *);
extern void queuePop16(void *, int16_t *);
extern void queuePop32(void *, int32_t *);
extern void queuePop64(void *, int64_t *);

extern void stageExecuter(void (*stage)(void *, void *), void *, void *);
extern DispatcherInfo NOELLE_DSWPDispatcher(void *env,
                                            int64_t *queueSizes,
                                            void *stages,
                                            int64_t numberOfStages,
                                            int64_t numberOfQueues);

extern void HELIX_wait(void *);
extern void HELIX_signal(void *);
extern DispatcherInfo NOELLE_HELIX_dispatcher_criticalSections(
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

extern DispatcherInfo NOELLE_HELIX_dispatcher_sequentialSegments(
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

extern uint32_t NOELLE_getAvailableCores(void);

void SIMONE_CAMPANONI_IS_GOING_TO_REMOVE_THIS_FUNCTION(void) {
  queuePush8(0, 0);
  queuePush16(0, 0);
  queuePush32(0, 0);
  queuePush64(0, 0);

  queuePop8(0, 0);
  queuePop16(0, 0);
  queuePop32(0, 0);
  queuePop64(0, 0);

  stageExecuter(0, 0, 0);
  NOELLE_DSWPDispatcher(0, 0, 0, 0, 0);

  NOELLE_HELIX_dispatcher_criticalSections(0, 0, 0, 0, 0);
  NOELLE_HELIX_dispatcher_sequentialSegments(0, 0, 0, 0, 0);
  HELIX_wait(0);
  HELIX_signal(0);

  int s;
  rand_r(&s);
  NOELLE_DOALLDispatcher(0, 0, 0, 0);

  NOELLE_getAvailableCores();
}
