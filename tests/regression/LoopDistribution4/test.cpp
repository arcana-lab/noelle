#include <stdio.h>

int inner(int iters) {
  return 1;
}

int main (int argc, char *argv[]){
  int x = 0;
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < argc; j++) {
      x += inner(j);
    }
  }
  printf("%d\n", x);
  return 0;
}

