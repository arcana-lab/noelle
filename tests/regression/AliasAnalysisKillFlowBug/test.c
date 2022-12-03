#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 200000

int main(int argc, char **argv) {
  unsigned freq;
  if (argc < 2) {
    printf("%s STRIDE\n", argv[0]);
    return 1;
  }
  freq = atoi(argv[1]);
  if (freq >= ARRAY_SIZE) {
    freq = 0;
  }

  int data[ARRAY_SIZE];
  memset(data, 0, sizeof(int));
  int sum = 0;
  for (int i = 0; i < (ARRAY_SIZE - freq); i++) {
    data[i + freq] = i * ARRAY_SIZE;
    sum += data[i];
  }
  printf("%d\n", sum);

  return 0;
}
