#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]){
  int v1, v2;
  v1 = atoi(argv[1]);
  v2 = atoi(argv[2]);

  for (int i = 0; i < 10000; ++i) {
    v1 = v1 * i;
    v2 = v1;
    while (v2 > 0){
      if (rand() > 1543){
        return 0;
      }
      v2--;
    }
  }

  printf("%d, %d\n", v1, v2);
  return 0;
}

