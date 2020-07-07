#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void computeSum (long long int *a, long long int SIZE_Z, long long int SIZE_X, long long int SIZE_Y){
	int x,  y,  z;

	for( z = -2; z < SIZE_Z+2; z++ ) {
		for( y = 0; y < SIZE_Y; y++ ) {
			for( x = 0; x < SIZE_X; x++ ) {
				if( x == 0 || x == SIZE_X-1 ||
				    y == 0 || y == SIZE_Y-1 ||
				    z == 0 || z == SIZE_Z-1 ) {
					(*a) += x + y + z;
				} else {
					if( (z == 1 || z == SIZE_Z-2) &&
					     x > 1 && x < SIZE_X-2 &&
					     y > 1 && y < SIZE_Y-2 ) {
					  (*a) -= x + y - z;
					}
				}
			}
		}
	}
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

  iterations *= 10;
  long long int *array = (long long int *) calloc(1, sizeof(long long int) * iterations);
  auto boundedArgc = argc < iterations ? argc : iterations - 5;
  array[boundedArgc] = argc;

  computeSum(array, iterations, 2, 2);
  auto s = *array;
  printf("%lld, %lld\n", s, array[boundedArgc]);

  return 0;
}
