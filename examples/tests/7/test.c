#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int32_t **allocateMatrix(int32_t l) {
  int32_t **p = (int32_t **)malloc(sizeof(int32_t *) * l);
  for (int32_t i = 0; i < l; i++) {
    p[i] = (int32_t *)malloc(sizeof(int32_t) * l);
    for (int32_t j = 0; j < l; j++) {
      p[i][j] = i + j;
    }
  }

  return p;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    return 1;
  }
  int32_t l = atoi(argv[1]);
  int32_t **m = allocateMatrix(l);

  for (int32_t i = 0; i < l; i++) {
    for (int32_t j = 0; j < l; j++) {
      printf("[%d][%d] = %d\n", i, j, m[i][j]);
    }
  }

  for (int32_t i = 0; i < l; i++) {
    free(m[i]);
  }
  free(m);

  return 0;
}
