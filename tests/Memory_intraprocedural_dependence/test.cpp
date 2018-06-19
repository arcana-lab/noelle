#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void myF (int *p_v1, int v2, long long int iterations, long long int scc1Iterations, long long int scc2Iterations){

  for (auto i = 0; i < iterations; ++i) {

    // SCC 1
    for (auto iter = 0; iter < scc1Iterations; iter++){
      (*p_v1) = (*p_v1) * 3;
      (*p_v1) = (*p_v1) / 2;
    }
    int v1_temp = *p_v1;

    // SCC 2
    v2 = v2 - v1_temp;
    for (auto iter = 0; iter < scc2Iterations; iter++){
      v2 = v2 * 3;
      v2 = v2 / 2;
    }
  }

  printf("%d, %d\n", *p_v1, v2);

  return ;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 4){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS SCC1_ITERATIONS SCC2_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  auto scc1Iterations = atoll(argv[2]);
  auto scc2Iterations = atoll(argv[3]);
  printf("SCC1 iterations = %lld\n", scc1Iterations);
  printf("SCC2 iterations = %lld\n", scc2Iterations);

  int v1, v2, v1_temp;
  v1 = argc;
  v2 = ((argc * 5) / 2) + 1;

  myF(&v1, v2, iterations, scc1Iterations, scc2Iterations);

  return 0;
}
