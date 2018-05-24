#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct _N {
  int v;
  _N *next;
} N;

int heavyComputation (int v){
  double d = (double)v;
  d += 0.143;
  d = sqrt(d);

  v = (int)d;

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
