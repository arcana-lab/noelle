#include <stdio.h>

int main (int argc, char *argv[]){
  int v1 = 0, v2 = 0, v3 = 0;

  for (int i = 0; i < 100; ++i) {
    v1 = v1 + argc;
    v2 = v1 + 5;

    if (i < argc) {
      v3 = v1 - argc;
    }
  }

  printf("%d, %d, %d\n", v1, v2, v3);
  return 0;
}