#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <functional>

std::function<void (int)> fetchF (void){
  auto myF = [](int n){
    if (n >= 1){
      exit(0);
    }
  };

  return myF;
}

int main (int argc, char *argv[]){
  int v1, v2;
  v1 = argc;
  v2 = ((argc * 5) / 2) + 1;

  auto g = fetchF();

  for (uint32_t i = 0; i < 100; ++i) {

    // SCC 1
    v1 = v1 + 1;
    v1 = v1 - 3;

    g(argc);

    // SCC 2
    v2 = v2 * 3 + v1;
    v2 = v2 / 2;
  }

  printf("%d, %d\n", v1, v2);
  return 0;
}
