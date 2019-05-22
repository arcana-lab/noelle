#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 10){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS ", argv[0]);
    for (auto i = 0; i < 8; i++){
      fprintf(stderr, "SCC%d_ITERATIONS ", i);
    }
    fprintf(stderr, "\n");
    return -1;
  }
  auto iterations = atoll(argv[1]);
  int64_t sccIterations[8];
  for (auto i = 0; i < 8; i++){
    sccIterations[i] = atoll(argv[i+2]);
  }

  auto v1 = argc;
  auto v2 = ((argc * 5) / 2) + 1;
  auto v3 = ((argc * 42) / 2) + 1;
  auto v4 = ((argc * 142) / 2) + 1;
  auto v5 = ((argc * 3) / 2) + 1;
  auto v6 = ((argc * 17) / 2) + 1;
  auto v7 = ((argc * 11) / 2) + 1;
  auto v8 = ((argc * 21) / 2) + 1;

  for (auto i = 0; i < iterations; ++i) {

    // SCC 1
    for (auto iter = 0; iter < sccIterations[0]; iter++){
      v1 = v1 * 3;
      v1 = v1 / 2;
    }

    // SCC 2
    for (auto iter = 0; iter < sccIterations[1]; iter++){
      v2 = v2 * v1;
      v2 = v2 / 2;
    }

    // SCC 3
    for (auto iter = 0; iter < sccIterations[2]; iter++){
      v3 = v3 * v2;
      v3 = v3 / 2;
    }

    // SCC 4
    for (auto iter = 0; iter < sccIterations[3]; iter++){
      v4 = v4 * v3;
      v4 = v4 / 2;
    }

    // SCC 5
    for (auto iter = 0; iter < sccIterations[4]; iter++){
      v5 = v5 * v4;
      v5 = v5 / 2;
    }

    // SCC 6
    for (auto iter = 0; iter < sccIterations[5]; iter++){
      v6 = v6 * v5;
      v6 = v6 / 2;
    }

    // SCC 7
    for (auto iter = 0; iter < sccIterations[6]; iter++){
      v7 = v7 * v6;
      v7 = v7 / 2;
    }

    // SCC 8
    for (auto iter = 0; iter < sccIterations[7]; iter++){
      v8 = v8 * v7;
      v8 = v8 / 2;
    }

  }

  printf("%d, %d, %d, %d, %d, %d, %d, %d\n", v1, v2, v3, v4, v5, v6, v7, v8);

  return 0;
}
