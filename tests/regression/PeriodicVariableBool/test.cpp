#include <stdio.h>
#include <stdlib.h>
#include <iostream>

int main(int argc, char *argv[]) {
  auto ntimes = atoll(argv[1]);
  bool flag = false;
  int i = 0;
  int acc = 0;

  while (i < ntimes) {
    flag = !flag;
    acc += flag;
    i++;
  }
  std::cout << "acc: " << acc << std::endl;

  return 0;
}
