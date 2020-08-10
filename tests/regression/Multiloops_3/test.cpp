#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 3){
    fprintf(stderr, "USAGE: %s ITERATIONS SEED\n", argv[0]);
    return 1;
  }

  /*
   * Fetch the inputs.
   */
  auto input = atoll(argv[1]);
  if (input < 1){
    input = 1;
  }
  auto iterations = input * 100;
  auto outerIterations = input;
  if (outerIterations > 10){
    outerIterations = 20;
  }
  auto seed = atoll(argv[2]);

  int *ar = (int *)malloc(sizeof(int) * iterations);
  for (auto i=0; i < outerIterations; i++){
    if (seed == 14324){
      printf("Hello\n");
    }
    
    float f = (float)(i * rand());
    f = sqrt(f);

    for (auto j=0; j < iterations; j++){
      ar[j] += i * seed;
      ar[j] += i * 23;
      ar[j] /= 2 + seed;
      ar[j] /= 3 - seed;
      ar[j] *= 41;
      ar[j] /= 2 + seed;
      ar[j] /= 3 - seed;
      ar[j] *= 41;
      ar[j] /= 2 + seed;
      ar[j] /= 3 - seed;
      ar[j] *= 41;
      ar[j] += i * 23;
      ar[j] /= 2 + seed;
      ar[j] /= 3 - seed;
      ar[j] *= 41;
      ar[j] /= 2 + seed;
      ar[j] /= 3 - seed;
      ar[j] *= 41;
      ar[j] /= 2 + seed;
      ar[j] /= 3 - seed;
      ar[j] *= 41;
    }

    for (auto j=0; j < iterations; j++){
      ar[j] += i * seed;
      ar[j] += i * 23;
      ar[j] /= 2 + seed;
      ar[j] /= 3 - seed;
      ar[j] *= 41;
      ar[j] += i * seed;
      ar[j] += i * 23;
      ar[j] /= 2 + seed;
      ar[j] /= 3 - seed;
      ar[j] *= 41;
      ar[j] /= 2 + seed;
      ar[j] /= 3 - seed;
      ar[j] *= 41;
      ar[j] /= 2 + seed;
      ar[j] /= 3 - seed;
      ar[j] *= 41;
      ar[j] += i * 23;
      ar[j] /= 2 + seed;
      ar[j] /= 3 - seed;
      ar[j] *= 41;
      ar[j] /= 2 + seed;
      ar[j] /= 3 - seed;
      ar[j] *= 41;
      ar[j] /= 2 + seed;
      ar[j] /= 3 - seed;
      ar[j] *= 41;
    }

  }
 
  printf("%lld\n", ar[iterations/2]);

  return 0;
}
