#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  auto ntimes = atoll(argv[1]);
  bool flag = false;
  int i = 0;

  while (i < ntimes) {
    printf("i: %d flag: %d\n", i, flag);
    flag = !flag;
    i++;
  }

  return 0;
}
