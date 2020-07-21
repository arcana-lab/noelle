#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void computeSum (long long int *a, long long int SIZE_Z, long long int SIZE_X, long long int SIZE_Y){
	int x,  y,  z, s;

	for( z = 0; z < SIZE_Z; z++ ) {
		for( y = 0; y < SIZE_Y; y++ ) {
			for( x = 0; x < SIZE_X; x++ ) {

        int valueToAdd = x + y - z;
        for (s = 0; s < SIZE_Z * SIZE_X * SIZE_Y; ++s) {
          valueToAdd += sqrt(s);
        }
        if (valueToAdd > 20) valueToAdd = sqrt(x + y - z);

				if( x == 0 || x == SIZE_X-1 ||
				    y == 0 || y == SIZE_Y-1 ||
				    z == 0 || z == SIZE_Z-1 ) {
          int offset = x + SIZE_X * y + z * (SIZE_X * SIZE_Y);
					a[offset] += x + y + z + valueToAdd;
				} else {
					if( (z == 1 || z == SIZE_Z-2) &&
					     x > 1 && x < SIZE_X-2 &&
					     y > 1 && y < SIZE_Y-2 ) {
            int offset = x + SIZE_X * y + z * (SIZE_X * SIZE_Y);

					  a[offset] += x + y - z + valueToAdd;
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

  long long int *array = (long long int *) calloc(iterations * iterations * iterations, sizeof(long long int));
  if (array == NULL){
    fprintf(stderr, "Cannot allocate memory\n");
    return 0;
  }
  auto nearEnd = (iterations * iterations * iterations) - 5;
  if (nearEnd < 0) nearEnd = 0;
  array[nearEnd] = argc;

  computeSum(array, iterations, iterations, iterations);
  auto s = *array;
  printf("%lld, %lld\n", s, array[nearEnd]);

  return 0;
}
