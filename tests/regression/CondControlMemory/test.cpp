#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int dont = 0;
void iPreventTheBlockFromCollapsingAlso () ;
void iPreventTheBlockFromCollapsing() {
  if (dont) iPreventTheBlockFromCollapsingAlso();
}
void iPreventTheBlockFromCollapsingAlso() {
  if (dont) iPreventTheBlockFromCollapsing();
}

int *z;
int main (int argc, char *argv[]) {

  if (argc < 2){
    fprintf(stderr, "USAGE: %s ITERATIONS\n", argv[0]);
    return -1;
  }

  auto iterations = atoi(argv[1]);

  int x = 3;
  z = (int *)calloc(1, sizeof(int32_t));
  for (int i = 0; i < iterations; ++i) {
    int xPlus;
    if (x < argc * 3) {
      xPlus = 1;
      iPreventTheBlockFromCollapsing();
    } else {
      xPlus = 2;
      iPreventTheBlockFromCollapsingAlso();
    }

    if (xPlus == 1) {
      xPlus += 5;
      z[0] += 3;
    } else {
      xPlus += 4;
      z[0] -= 7;
    }

    x += xPlus;
  }

  printf("value = %d, %d, %d\n", x, z[0], z[1]);

  return 0;
}

