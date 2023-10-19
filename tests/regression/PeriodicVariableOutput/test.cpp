#include <stdio.h>
#include <stdlib.h>
#include <iostream>

int main(int argc, char *argv[]) {
  auto ntimes = atoll(argv[1]);
  bool flag = false;
  int i = 0;

  while (i < ntimes) {
    std::cout << "i: " << i << " flag: " << flag << std::endl;
    flag = !flag;
    i++;
  }

  return 0;
}