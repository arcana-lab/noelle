#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int myFunction (int argc, long long int scc1Iterations, long long int scc2Iterations, long long int iterations){
  int v1, v2;
  v1 = argc;
  v2 = ((argc * 5) / 2) + 1;

  for (auto i = 0; i < iterations; ++i) {

    v1 = v1 * 3;
    v1 = v1 / 2;

    if (v1 > 200){
      printf("%d, %d\n", v1, v2);
      return 42;
    }
    if (v1 == 109) {
      return 43;
    }

    v2 = v2 - v1;
    v2 = v2 * 3;
    v2 = v2 / 2;
  }

  printf("%d, %d\n", v1, v2);
  return v1;
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

  /*
   * Run
   */
  printf("INVOCATION: %d\n", myFunction(argc, scc1Iterations, scc2Iterations, iterations));
  printf("INVOCATION: %d\n", myFunction(1, 1, 1, 1));
  printf("INVOCATION: %d\n", myFunction(10, 10, 10, 10));
  printf("INVOCATION: %d\n", myFunction(100, 100, 100, 100));

  return 0;
}
