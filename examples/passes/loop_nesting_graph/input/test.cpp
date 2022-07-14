#include "stdio.h"
#include "stdlib.h"
#define N 10000
#define M 10000

int foo(int *a) {
  int sum = 0;

  // loop C
  for (int i = 0; i < M; i++) {
    sum += a[i];
    sum %= 101;
  }
  return sum;
}

int foo2(int *a) {
  int sum = 0;

  // loop C
  for (int i = 0; i < M; i++) {
    sum += a[i];
    sum %= 103;
  }
  return sum;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: %s use_foo2(0/1)\n", argv[0]);
    return 1;
  }
  bool use_foo2 = atoi(argv[1]);
  int sum = 0;

  int *arr = new int[M];
  // loop A.0
  for (int i = 0; i < N; i++) {
    // loop A.1
    for (int j = 0; j < M; j++) {
      arr[j] = i * j % 7;
    }

    sum = foo(arr);
  }

  int (*foo_ptr)(int *);

  if (use_foo2) {
    foo_ptr = foo2;
  } else {
    foo_ptr = foo;
  }

  // loop B.0
  for (int i = 0; i < N; i++) {
    // loop B.1
    for (int j = 0; j < M; j++) {
      arr[j] = i * j % 13;
    }

    sum = foo_ptr(arr);
  }
}
