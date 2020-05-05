#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main (int argc, char *argv[]) {
  if (argc < 2){
    fprintf(stderr, "USAGE: %s SCC1_CONTROL_FLOW SCC2_CONTROL_FLOW\n", argv[0]);
    return -1;
  }

  auto scc1cf = atoi(argv[1]);
  auto scc2cf = atoi(argv[2]);

  int x = 3;
  int y = 5;
  int z = 10;
  for (int i = 0; i < argc * 1000; ++i) {
    x += 1;
    if (scc1cf > x) {
      x += 1;
      if (scc1cf > 10 * x) {
        x *= 1.1;
        y -= 10;
        if (scc2cf > y) {
          z += 5;
        }
      }
    }

    y -= 1;
    if (scc2cf > y) {
      y -= 1;
      if (scc2cf > 10 * y) {
        y /= 1.1;
      }
    }
  }

  printf("SCC1 value = %d, SCC2 value = %d, SCC3 value = %d\n", x, y, z);

  return 0;
}
