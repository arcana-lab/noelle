#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long long int computeSum (long long int *a, long long int iters, long long int seed){
  long long int s =0;

  int j = 0;
  long long int i=0;
  do {
    if (i > 500000) break ;

    a[i] += j + 2;
    a[i] /= (j + 1);
    auto z = (((j * 3) % 142) / 2) + 1;
    if (z == 0){
      z++;
    }
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z * 1;
    a[i] /= z;
    if (i > 1000000) break ;
    z = (((j * 3) % 142) / 2) + 1;
    if (z == 0){
      z++;
    }
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    if (i > 2000000) break ;
    z = (((j * 3) % 142) / 2) + 1;
    if (z == 0){
      z++;
    }

    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    if (i > 500000) break ;
    z = (((j * 3) % 142) / 2) + 1;
    if (z == 0){
      z++;
    }

    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    if (i > 2000000) break ;
    z = (((j * 3) % 142) / 2) + 1;
    if (z == 0){
      z++;
    }

    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    if (i > 500000) break ;
    z = (((j * 3) % 142) / 2) + 1;
    if (z == 0){
      z++;
    }

    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    if (i > 2000000) break ;
    z = (((j * 3) % 142) / 2) + 1;
    if (z == 0){
      z++;
    }

    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    if (i > 500000) break ;
    z = (((j * 3) % 142) / 2) + 1;
    if (z == 0){
      z++;
    }

    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    if (i > 2000000) break ;
    z = (((j * 3) % 142) / 2) + 1;
    if (z == 0){
      z++;
    }

    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    if (i > 500000) break ;
    z = (((j * 3) % 142) / 2) + 1;
    if (z == 0){
      z++;
    }

    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    if (i > 2000000) break ;
    z = (((j * 3) % 142) / 2) + 1;
    if (z == 0){
      z++;
    }

    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    if (i > 500000) break ;
    z = (((j * 3) % 142) / 2) + 1;
    if (z == 0){
      z++;
    }

    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    if (i > 2000000) break ;
    z = (((j * 3) % 142) / 2) + 1;
    if (z == 0){
      z++;
    }

    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    if (i > 500000) break ;
    z = (((j * 3) % 142) / 2) + 1;
    if (z == 0){
      z++;
    }

    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;
    a[i] /= z;

    i++;
    j += i;

  } while (i < iters);

  return s;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 3){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS SEED\n", argv[0]);
    return -1;
  }
  auto iterations = atoll(argv[1]);
  if (iterations == 0) return 0;
  iterations *= 100;
  auto seed = atoll(argv[2]);

  long long int *array = (long long int *) calloc(iterations, sizeof(long long int));
  for (auto i=0; i < iterations; i++){
    array[i] = i * 5;
  }

  auto s = computeSum(array, iterations, seed);
  long long int t=0;
  for (auto i=0; i < iterations; i++){
    t += array[i];
  }
  printf("%lld %lld %lld\n", s, t, array[iterations/2]);

  return 0;
}
