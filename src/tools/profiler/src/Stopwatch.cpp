extern "C" {

#include <stdbool.h>
#include <stdio.h>
#include <time.h>

typedef struct {
  struct timespec start_time;
  double elapsed_s;
  bool running;
  long laps;
  const char *name;
} stopwatch_t;

double stopwatch_timespec_diff_ms(const struct timespec *end,
                                  const struct timespec *start) {
  return (end->tv_sec - start->tv_sec) * 1.0
         + (end->tv_nsec - start->tv_nsec) / 1e9;
}

void stopwatch_init(stopwatch_t *sw, const char *name) {
  sw->elapsed_s = 0.0;
  sw->running = false;
  sw->name = name;
  sw->laps = 0;
}

double stopwatch_milliseconds(const stopwatch_t *sw) {
  return sw->elapsed_s * 1000.0;
}

double stopwatch_seconds(const stopwatch_t *sw) {
  return sw->elapsed_s;
}

void stopwatch_print(const stopwatch_t *sw) {
  double sec = stopwatch_seconds(sw);
  printf("[noelle.stopwatch] %s: %.3f s\n", sw->name, sec);
}

void stopwatch_print_stats(const stopwatch_t *sw) {
  double sec = stopwatch_seconds(sw);
  printf("[noelle.stopwatch] %s: %.3f s (avg: %.3f s, laps: %li)\n",
         sw->name,
         sec,
         sec / sw->laps,
         sw->laps);
}

void stopwatch_start(stopwatch_t *sw) {
  if (!sw->running) {
    clock_gettime(CLOCK_MONOTONIC, &sw->start_time);
    sw->running = true;
  }
}

void stopwatch_stop(stopwatch_t *sw) {
  if (sw->running) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    sw->elapsed_s += stopwatch_timespec_diff_ms(&now, &sw->start_time);
    sw->running = false;
    ++sw->laps;
  }
}

void stopwatch_reset(stopwatch_t *sw) {
  sw->elapsed_s = 0.0;
  sw->running = false;
}
}
