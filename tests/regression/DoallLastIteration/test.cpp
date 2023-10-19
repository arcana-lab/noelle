#include <stdio.h>
#include <stdlib.h>
#include <iostream>

int main(int argc, char *argv[]) {
  int *p = (int *)malloc(101 * sizeof(int));
  int *q = (int *)malloc(101 * sizeof(int));
  int *r = (int *)malloc(101 * sizeof(int));

  int *old_p = p;
  for (int i = 0; i < 100; i++) {
    p++;
    *p = 42;
  }
  *p = 43;
  std::cout << "*p:  " << *p << std::endl;
  std::cout << "p address diff: " << (p - old_p) * sizeof(int) << std::endl;

  int *old_q = q;
  for (int i = 100; i >= 1; i--) {
    q++;
    *q = 42;
  }
  *q = 43;
  std::cout << "*q:  " << *q << std::endl;
  std::cout << "q address diff: " << (q - old_q) * sizeof(int) << std::endl;

  int *old_r = r;
  for (int i = 0; i < 199; i += 2) {
    r++;
    *r = 42;
  }
  *r = 43;
  std::cout << "*r:  " << *r << std::endl;
  std::cout << "r address diff: " << (r - old_r) * sizeof(int) << std::endl;

  return 0;
}
