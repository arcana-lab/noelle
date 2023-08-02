#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>

#define CACHE_LINE_SIZE 64

int main(int argc, char *argv[]) {
  uint64_t VSIZE = (uint64_t)atoi(argv[1]);
  uint64_t LOOP_ITER = (uint64_t)atoi(argv[2]);

  void *x = nullptr;
  posix_memalign(&x, CACHE_LINE_SIZE, CACHE_LINE_SIZE * VSIZE);
  memset(x, 0, CACHE_LINE_SIZE * VSIZE);

  for (uint64_t i = 0; i < LOOP_ITER; ++i) {
    (*((int *)(((uint64_t)x) + CACHE_LINE_SIZE * (i % VSIZE)))) += i;
  }

  printf("%d\n", *((int *)(((uint64_t)x) + CACHE_LINE_SIZE * (1 % VSIZE))));

  return 0;
}
