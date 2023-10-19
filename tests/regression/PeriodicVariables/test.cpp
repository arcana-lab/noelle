#include <stdio.h>
#include <stdlib.h>
#include <iostream>

int main(int argc, char *argv[]) {
  auto ntimes = atoll(argv[1]);
  ntimes *= 100000;

  int flag1 = 0;
  bool flag2 = false;
  int flag3 = 10;

  int i = 0;
  uint64_t acc = 0;
  while (i < ntimes) {
    flag1 ^= 1;
    flag2 = !flag2;
    flag3 = -flag3;
    acc += flag1 + flag2 + flag3;
    i++;
  }
  std::cout << " acc: " << acc << std::endl;

  return 0;
}
