#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int heavyComputation (int v){

  for (int i=0; i < 1000; i++){
    for (int j=0; j < 1000; j++){
      double d = (double)v;
      d += 0.143;
      for (int z=0; z < 10; z++){
        d = sqrt(d);
      }

      v = (int)d;
    }
  }

  return v;
}

int main (){
  printf("%d\n", heavyComputation(42));

  return 0;
}
