#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

struct ThingOfPrimitives {
  char charOne;
  short shortOne;
  int64_t justRightOne;
  long long int reallyBigOne;
};

void writeTo(int &i, int v) {
  i = v + 1;
}

int main (int argc, char *argv[]){
  if (argc < 1) return 0;

  int iterations = 10 * std::fmax(argc, 3);
  int iterations2 = (iterations / 2);
  // ThingOfPrimitives * matrix = (ThingOfPrimitives *)calloc(iterations * iterations2, sizeof(ThingOfPrimitives));

  /*
   * Clonable
   */
  ThingOfPrimitives thingWrittenToByMemcpy;
  ThingOfPrimitives thingWrittenToElementByElement;

  /*
   * Not clonable
   */
  ThingOfPrimitives notFullyStoredToThing;
  ThingOfPrimitives mightNotBeResetEachIteration;
  notFullyStoredToThing.justRightOne = 2;
  mightNotBeResetEachIteration.justRightOne = 2;

  int totalValue = 0;

  for (int i = 0; i < iterations; ++i) {
    notFullyStoredToThing.reallyBigOne = i + 1;

    if (i > 10) {
      int j = 0;
      mightNotBeResetEachIteration.charOne = (char)i;
      mightNotBeResetEachIteration.shortOne = i + j;
      mightNotBeResetEachIteration.justRightOne = i * i + j * j;
      mightNotBeResetEachIteration.reallyBigOne = i * i + j * (i - 1) + i * j * (i - 2) * (j - 2);
    }

    for (int j = 0; j < iterations2; ++j) {

      /*
       * This sets the integer type, hence myV is clonable
       */
      int myV = i;
      writeTo(myV, myV + 4);

      /*
       * These setters cover all fields, so it is clonable
       */
      thingWrittenToElementByElement.charOne = (char)i;
      thingWrittenToElementByElement.shortOne = i + j;
      thingWrittenToElementByElement.justRightOne = i * i + j * j + myV;
      thingWrittenToElementByElement.reallyBigOne = i * i + j * (i - 1) + i * j * (i - 2) * (j - 2);

      /*
       * This fully stores a struct into thing, hence it is clonable
       */
      if (j > 10) {
        thingWrittenToByMemcpy = thingWrittenToElementByElement;
        totalValue += thingWrittenToByMemcpy.justRightOne;
      }
    }

    totalValue += notFullyStoredToThing.justRightOne;
    totalValue += mightNotBeResetEachIteration.justRightOne;
  }

  printf("%d\n", totalValue);

  return 0;
}