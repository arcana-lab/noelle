#include <stdio.h>
#include <stdint.h>

int main (int argc, char *argv[]){
  double v1, v2;
  v1 = argc + 0.31;
  v2 = ((argc * 5) / 2) + 1.5324;
  int reset = argc > 100 ? 0 : 1;

  for (uint32_t i = 0; i < 100; ++i) {

    // SCC 1
    for (uint32_t j = 0; j < 10; j++){
      double temp = (((double)j) * 25.210);
      v1 += temp;
    }

    // SCC 2
    v2 = v2 + v1;
  }

  printf("%f, %f\n", v1, v2);
  return 0;
}
