#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  int ni = atoi(argv[1]);
  int alpha = 2;

  int *tmp = (int *)calloc(ni, sizeof(int));
  for (int i = 0; i < ni; ++i) {
    printf("%d", tmp[i]);
  }
  printf("\n");

  free((void *)tmp);
  return 0;
}
