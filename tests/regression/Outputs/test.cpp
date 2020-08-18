#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 2){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoi(argv[1]);
  if (iterations < 1){
    iterations = 1;
  }
  iterations *= 100;

  unsigned long long int t = 1;
  for (auto i=0; i < iterations; i++){
    fprintf(stderr, "Start iteration %d\n", i);
    auto v = sqrt((double)iterations);
    v += iterations;
    v = sqrt((double)iterations);
    v += iterations;
    v = sqrt((double)iterations);
    v += iterations;
    v = sqrt((double)iterations);
    v += iterations;
    v = sqrt((double)iterations);
    v += iterations;
    v = sqrt((double)iterations);
    v += iterations;
    v = sqrt((double)iterations);
    v += iterations;
    v = sqrt((double)iterations);
    v += iterations;
    v = sqrt((double)iterations);
    v += iterations;
    v = sqrt((double)iterations);
    v += iterations;
    v = sqrt((double)iterations);
    v += iterations;
    v = sqrt((double)iterations);
    t *= (unsigned long long int)v;
    fprintf(stderr, "End iteration %d\n", i);
  }

  fprintf(stderr, "Total = %lld\n", t);

  return 0;
}
