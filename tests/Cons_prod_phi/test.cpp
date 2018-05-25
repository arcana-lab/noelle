#include <stdio.h>
#include <stdint.h>

int main (int argc, char *argv[]){
  int v1, v2, v1_temp, v3;
  v1 = argc;
  v2 = ((argc * 5) / 2) + 1;
  v3 = 3;

  for (uint32_t i = 0; i < 100; ++i) {
    v3 = v2;

    // SCC 1
    v1_temp = v1;
    v1 = v1 + 4;

    // SCC 2
    v2 = v2 - v1_temp;
    v2 = v2 * 3;
    v2 = v2 / 2;

    if (v1 > 10) {
      v1 = v2 - 4;
      break;
    } else {
      v1 += 2;
    }
  }

  v2 += v3;
  printf("%d, %d\n", v1, v2);
  return 0;
}
