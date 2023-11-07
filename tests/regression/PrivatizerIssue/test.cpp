#include <stdio.h>
#include <stdlib.h>

int globalArray[10];

int main(int argc, char *argv[]) {
  if (argc < 3) {
    fprintf(stderr, "USAGE: x, y, z must be provided.\n");
    return -1;
  }

  auto x = atoi(argv[1]);
  auto y = atoi(argv[2]);
  auto z = atoi(argv[3]);

  for (int i = 0; i < 10; i++) {
    globalArray[i] = x;
    if (i > 3) {
      printf("i>3\n");
      globalArray[i] = y;
      if (i < 7) {
        printf("i<7\n");
        globalArray[i] = z;
      } else {
        printf("i>=7\n");
      }
    }
  }

  return 0;
}
