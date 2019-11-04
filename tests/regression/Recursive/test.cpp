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

int nesta (int);
int nestb (int);
int nestc (int);
int nestd (int);
int neste (int);
int nestf (int);
int nestg (int);

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

  /*
   * Example of more nested recursion
   */
  printf("%d\n", nesta(iterations / 10));

  return 0;
}

/*
 * The call graph for these nest functions:
 * A -> B -> F -> G -> C ...
 *   -> C -> D -> E -> C ...
 * Depth based pre order would be A, B, C, F, D, G, E
 */
int nesta (int iters) {
  if (iters == 0) return 0;
  int x = 0, y = 0, z = 0;
  for (int i = 0; i < 10; ++i) {
    x += nestb(iters - 1);
    y += nestc(iters - 1);
    z += nestb(iters - 1);
  }
  return x + y + z;
}

int nestb (int iters) {
  if (iters == 0) return 0;
  int x = nestf(iters - 1);
  return x;
}

int nestc (int iters) {
  if (iters == 0) return 0;
  int x = nestd(iters - 1);
  return x;
}

int nestd (int iters) {
  if (iters == 0) return 0;
  int x = neste(iters - 1);
  return x;
}

int neste (int iters) {
  if (iters == 0) return 0;
  int x = nestc(iters - 1);
  return x;
}

int nestf (int iters) {
  if (iters == 0) return 0;
  int x = 0;
  for (int i = 0; i < iters; i++) {
    for (int j = 0; j < i; j++) {
      x += nestg(i - j);
    }
    for (int j = 0; j < i; j++) {
      x += nestg(i - j);
    }
  }
  return x;
}

int nestg (int iters) {
  if (iters == 0) return 0;
  int x = 0;
  for (int i = 0; i < 10; ++i) {
    x += nestc(iters - 1);
    x -= nestc(iters - 2);
  }
  return x;
}
