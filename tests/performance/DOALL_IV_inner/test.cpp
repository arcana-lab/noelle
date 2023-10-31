#include <stdio.h>
#include <stdlib.h>
#include <iostream>

int main(int argc, char *argv[]) {
  auto ntimes = atoll(argv[1]);
  int multiplier = 500;
  int *p = (int *)malloc(ntimes * multiplier * 10 * sizeof(int));

  int *old_p = p;
  for (int i = 0; i < ntimes * multiplier; i++) {
    for (int j = 0; j < 10; j++) {
      p++;
      *p = 42;
    }
    *p = 43;
  }
  std::cout << "*p:  " << *p << std::endl;
  std::cout << "p address diff:  " << (p - old_p) * sizeof(int) << std::endl;
  return 0;
}
