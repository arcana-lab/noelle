#include <stdio.h>
#include <stdint.h>

int main (int argc, char *argv[]){
  int v1, v2;
  v1 = argc;
  for (uint32_t i = 0; i < 10000; ++i) {

    // SCC 1
    if (v1 < argc * 2) {
      for (int j = 0; j < argc; ++j) {
        v1 += 5;
      }
    } else {
      for (int j = 0; j < argc + 5; ++j) {
        v1 -= 3;
      }
    }
  }

  printf("%d, %d\n", v1, v2);
  return 0;
}
