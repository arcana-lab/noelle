#include <math.h>
#include <stdio.h>
#include <stdlib.h>

float heavyComputation(float input) {

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      auto d = input;
      d += 0.143;
      for (int z = 0; z < 3; z++) {
        d = sqrt(d);
      }

      input = d;
    }
  }

  return input;
}

int main() {
  auto v = heavyComputation(35.3);
  printf("%f\n", v);

  return 0;
}
