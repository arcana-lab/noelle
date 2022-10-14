#include <stdio.h>
#include <stdlib.h>

void print_array(int ni, int D[ni]) {
  int i;
  for (i = 0; i < ni; i++)
    fprintf(stderr, "%d ", D[i]);
  fprintf(stderr, "\n");
}

int main(int argc, char **argv) {
  int ni = atoi(argv[1]);
  int alpha = 2;

  int *tmp = (int *)calloc(ni, sizeof(int));
  for (int i = 0; i < ni; ++i) {
    tmp[0] += alpha;
  }

  print_array(ni, tmp);
  free((void *)tmp);
  return 0;
}
