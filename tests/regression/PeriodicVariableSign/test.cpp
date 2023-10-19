#include <stdio.h>
#include <stdlib.h>
#include <iostream>

int main(int argc, char *argv[]) {
  auto ntimes = atoll(argv[1]);
  int k = 1;
  int i = 0;
  int acc = 0;

  while (i < ntimes) {
    k = -k;
    acc -= k;
    i++;
  }
  std::cout << "acc: " << acc << std::endl;

  return 0;
}
