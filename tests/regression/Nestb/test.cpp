#include <stdio.h>
#include <stdlib.h>

int nestb(int) ;
int nestc(int) ;
int nestd(int) ;
int neste(int) ;

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  if (argc < 2){
    fprintf(stderr, "USAGE: %s LOOP_ITERATIONS\n", argv[0]);
    return -1;
  }
  auto iterations = atoi(argv[1]);

  /*
   * Example of more nested recursion
   */
  printf("%d\n", nestb(iterations / 10));

  return 0;
}

int nestb (int iters) {
  if (iters <= 0) return 0;
  int x = 0;
  for (int i = 0; i < iters; i++) {
    for (int j = 0; j < i; j++) {
      int k = i - j;
      int y = 0;
      if (k > 0) {
        for (int m = 0; m < 10; ++m) {
          y += nestc(iters);
          y -= nestc(iters - 5);
        }
      }
      x += y;
    }
    for (int j = 0; j < i; j++) {
      int k = i - j;
      int y = 0;
      if (k > 0) {
        for (int m = 0; m < 10; ++m) {
          y += nestc(iters);
          y -= nestc(iters - 5);
        }
      }
      x += y;
    }
  }
  return x;
}

int nestc (int iters) {
  if (iters <= 0) return 0;
  int x = nestd(iters - 1);
  return x + 1;
}

int nestd (int iters) {
  if (iters <= 0) return 0;
  int x = neste(iters - 1);
  return x + 1;
}

int neste (int iters) {
  if (iters <= 0) return 0;
  int x = nestc(iters - 1);
  return x + 1;
}
