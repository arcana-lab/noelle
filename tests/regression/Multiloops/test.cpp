#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct _N {
  int v;
  _N *next;
} N;

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
    printf("Progress: %d\n", v);
  }

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

  N *tmpN = n0;
  while (tmpN != NULL){
    int v = tmpN->v;

    if (v < 40){
      v = heavyComputation(v);
    }

    printf("%d\n", v);

    tmpN = tmpN->next;
  }

  return 0;
}
