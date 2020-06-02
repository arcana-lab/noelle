#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct _N {
  int v;
  _N *next;
} N;

namespace myM {
  int sqrt (int v) {
    if (v == 0)
      return v;
    return v + 1 / v;
  }
}

int heavyComputation (int v){
  double d = (double)v;
  double c = 0;
  for (auto i = 0; i < 10000; i++) {
    d += 0.143;
    d = std::sqrt(d);
    c = myM::sqrt(d);
  }

  v = (int)d + (int)c;

  return v;
}

void appendNode (N* tail, int newValue, int howManyMore){

  N *newNode = (N *) malloc(sizeof(N));
  newNode->v = newValue;
  newNode->next = NULL;

  tail->next = newNode ;

  if (howManyMore > 0){
    appendNode(newNode, newValue+1, howManyMore - 1);
  }

  return ;
}

int main (){
  N *n0 = (N *) malloc(sizeof(N));
  n0->v = 41;
  appendNode(n0, 42, 99);

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
