#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void computeSum (long long int *a, long long int SIZE_Z){
  for(int z = 0; z < SIZE_Z; z++){
    if (z == 0){
      for (int j=0; j < 1000000000; j++){
        rand();
      }
    }

    (*a) += z;
  }

  return ;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 2){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  if (iterations == 0) return 0;

  long long int *array = (long long int *) calloc(iterations, sizeof(long long int));

  computeSum(&(array[0]), iterations);
  auto s = array[0];

  printf("%lld %lld\n", s, array[iterations/2]);

  return 0;
}
