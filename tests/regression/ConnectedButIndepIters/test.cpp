#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long long int computePowerSeries (long long int *a, long long int iters){
  long long int s =0;
  long long int t =0;
  long long int u =0;
  long long int v =0;

  for (auto i=0; i < iters; ++i){
    s += a[i];
    for (long long int j=0; j < iters / 10; ++j){
      t += a[j];
    }
    u -= t;
    for (long long int j=0; j < iters / 10; ++j){
      u += a[j];
    }
    v -= u;
    for (long long int j=0; j < iters / 10; ++j){
      v += a[j];
    }
  }

  long long int x = s + t + u + v;
  return x;
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
  long long int *array = (long long int *) malloc(sizeof(long long int) * iterations);

  for (auto i=0; i < iterations; i++){
    array[i] = i;
  }

  auto s = computePowerSeries(array, iterations);
  printf("%lld\n", s);

  return 0;
}
