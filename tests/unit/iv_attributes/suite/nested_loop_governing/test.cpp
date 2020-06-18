#include <stdio.h>
#include <stdint.h>
#include <math.h>

int main (int argc, char *argv[]){

  if (argc < 4){
    fprintf(stderr, "USAGE: %s LOOPX_ITER LOOPY_ITER LOOPZ_ITER; passed %d arguments\n", argv[0], argc);
    return -1;
  }

  auto SIZE_X = atoll(argv[1]);
  auto SIZE_Y = atoll(argv[2]);
  auto SIZE_Z = atoll(argv[3]);

  long long int *a = (long long int *) calloc(0, sizeof(long long int) * SIZE_Z);

  int x, y, z;
  for (z = -2; z < SIZE_Z + 2; z++) {
    for (y = 0; y < SIZE_Y; y++) {
      for (x = 0; x < SIZE_X; x++) {
        if (x == 0 || x == SIZE_X - 1 ||
            y == 0 || y == SIZE_Y - 1 ||
            z == 0 || z == SIZE_Z - 1) {
          (*a) += x + y + z;
        }
        else {
          if ((z == 1 || z == SIZE_Z - 2) &&
              x > 1 && x < SIZE_X - 2 &&
              y > 1 && y < SIZE_Y - 2) {
            (*a) -= x + y - z;
          }
        }
      }
    }
  }

  printf("%lld\n", *a);
  return 0;
}
