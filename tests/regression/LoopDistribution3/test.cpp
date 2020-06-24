#include <stdio.h>

int main (int argc, char *argv[]){
  int x;
  int y = argc;

  for (int i = 0; i < 1000; ++i) {
    if (y % argc == 0) {
      y += 3;
      y %= 10;
    } else {
      y += 10;
      y *= 5;
    }
    x = i % 30;
    printf("Hello, world!");
  }
  printf("%d\n", x);
  printf("%d\n", y);

  return 0;
}

