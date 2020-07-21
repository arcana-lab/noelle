#include <stdio.h>
#include <stdlib.h>

typedef struct _N {
  int v;
  int z;
  _N *next;
} N;


N* appendNode (N* tail, int newValue, int howManyMore){

  N *newNode = (N *) malloc(sizeof(N));
  newNode->v = newValue;
  newNode->z = newValue + 1;
  newNode->next = NULL;

  tail->next = newNode ;

  if (howManyMore > 0){
    return appendNode(newNode, newValue+1, howManyMore - 1);
  }

  return newNode;;
}

int main (int argc, char *argv[]){
  N *first = (N *) malloc(sizeof(N));
  first->v = 3;
  first->z = 4;
  first->next = NULL;

  N *last = appendNode(first, 4, argc);
  N *new_next = (N *) malloc(sizeof(N));
  new_next->v = 10;
  new_next->z = 11;
  new_next->next = NULL;

  N *tmp = first;
  N *next = first->next;
  int prev_v = 5;

  int sum = 0;

  while (tmp != last) {
    sum += argc;
    sum *= argc;
    sum %= argc;
    sum += argc + 50;

    tmp->next = new_next;

    int temp_prev_v = tmp->v;
    tmp->v = prev_v + 2;
    prev_v = temp_prev_v;

    new_next = tmp;
    tmp = next;
    next = tmp->next;
  }
  printf("%d\n", sum);
  return 0;
}

