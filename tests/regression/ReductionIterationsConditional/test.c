#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

long long int computation (long long int iters, int *values, unsigned int numberOfValues, long long int valueToSet){
  long long int t = 0;

  for (auto i=0; i < iters; ++i){
    if (values[i % numberOfValues] >= 5){
      t = valueToSet;
    }
  }

  return t;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 3){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS VALUE+\n", argv[0]);
    return -1;
  }
  unsigned int iterations = atoll(argv[1]);
  iterations *= 10;

  unsigned int numberOfValues = argc - 2;
  printf("Number of values = %u\n", numberOfValues);
  assert(numberOfValues > 0);
  int *values = (int *)malloc(sizeof(int) * numberOfValues);
  for (auto i=0; i < numberOfValues; i++){
    values[i] = atoi(argv[i + 2]);
  }
  long long int valueToSet = atoi(argv[2]);

  long long int s = computation(iterations, values, numberOfValues, valueToSet);
  printf("%lld\n", s);

  return 0;
}
