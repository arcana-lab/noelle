#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float heavyComputation (float input, float input2){

  for (int i=0; i < 10; i++){
    if (i > 3) {
    for (int j=0; j < 10; j++){
      auto d = input;
      d += 0.143;
      for (int z=0; z < 3; z++){
        d = sqrt(d);
      }

      input = d;
    }
    }

    if (i == 9) input2 += input;

    if (i < 7) {
    for (int j=0; j < 5; j++) {
      auto e = input2; 
      e -= .198;
      e = sqrt(e);
/*      
      if (i == 6) {
        break;
      } else if (i == 5) {
        input = e;
      } else if (i > 2) {
        for (int k=0; k < 3; k++) {
          input2 -= k;
          if (i == 3) break;
        }
      }
*/
      input2 = e;
    }
    }
  }

  return input;
}

int main (){
  auto v = heavyComputation(35.3, 24.7);
  printf("%f\n", v);

  return 0;
}
