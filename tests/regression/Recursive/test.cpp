#include <stdio.h>
#include <stdlib.h>

int funcb (int *a, int iters) ;

int funca (int *a, int iters) {
  int count = 0;
  for (int i = 0; i < iters; ++i) {
    count += a[i];
    count += funcb(a, i);
  }
  return count;
}

int funcb (int *a, int iters) {
  int count = 0;
  for (int i = 0; i < iters; ++i) {
    count += funca(a, i);
    a[i] += count;
  }
  return count;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 2){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoi(argv[1]);

  int *array = (int *) malloc(sizeof(int) * iterations);
  for (auto i=0; i < iterations; i++){
    array[i] = i;
  }

  auto output = funca(array, iterations);
  printf("%d\n", output);

  return 0;
}
