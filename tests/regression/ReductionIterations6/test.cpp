#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void computeSum (long long int *a, long long int SIZE_Z, long long int SIZE_X, long long int SIZE_Y, int externalCondition){
	int x, y,  z;

  x = SIZE_X;
  y = SIZE_Y;
  for(z = 0; z < SIZE_Z; z++){
    if (z == 0){
      for (int j=0; j < 1000000; j++){
        rand();
      }
    }

    if (externalCondition){
      (*a) += x + y + z;
    }
  }

  return ;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 3){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS WRITE_TO_MEMORY\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  auto externalCondition = atoi(argv[2]);
  if (iterations == 0) return 0;

  iterations *= 100;
  long long int *array = (long long int *) calloc(iterations, sizeof(long long int));

  computeSum(&(array[0]), iterations, 2, 2, externalCondition);
  auto s = array[0];

  printf("%lld %lld\n", s, array[iterations/2]);

  return 0;
}
