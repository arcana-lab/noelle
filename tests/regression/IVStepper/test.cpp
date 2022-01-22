#include <stdio.h>
#include <stdlib.h>

int main (int argc, char* argv[]) {
  int step;
  if (argc < 2) {
    step = 1;
  } else {
    step = atoi(argv[1]);
  }

  int v1 = 0;

  for (int i = 0; i < 100; ++i) {
    v1 += step;
    printf("%d\n", v1);
  }

  printf("%d\n", v1);
  return 0;
}

