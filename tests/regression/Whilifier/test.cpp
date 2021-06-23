#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct _N {
  int v;
  _N *next;
} N;

int glob = 0;
int globalSqrt (int v) {
  if (++glob > 10) {
    glob = 0;
  }
  if (v == 0)
    return v;
  return v + 1042 / v;
}

int heavyComputation (int v){
  double e = 0;
  for (auto i = 0; i < 10000; i++) {
    e += globalSqrt(e);
  }
  v = (int)e;
  return v;
}

int main (){
  N *n0 = (N *) malloc(sizeof(N));
  n0->v = 41;
  N *newNode = (N *) malloc(sizeof(N));
  newNode->v = 42;
  newNode->next = NULL;
  n0->next = newNode ;

  int vSum = 0;
  N *tmpN = n0;
  while (tmpN != NULL){
    int v = tmpN->v;

    if (v < 40){
      v = heavyComputation(v);
    }

    vSum += v;

    tmpN = tmpN->next;
  }

  printf("%d\n", vSum);

  return 0;
}
