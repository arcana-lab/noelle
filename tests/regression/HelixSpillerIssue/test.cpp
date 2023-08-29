#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct _N {
  int v;
  _N *next;
} N;

void appendNode(N *tail, int newValue, int howManyMore) {

  N *newNode = (N *)malloc(sizeof(N));
  newNode->v = newValue;
  newNode->next = NULL;

  tail->next = newNode;

  if (howManyMore > 0) {
    appendNode(newNode, newValue + 1, howManyMore - 1);
  }

  return;
}

int main(int argc, char *argv[]) {

  N *n0 = (N *)malloc(sizeof(N));
  n0->v = 0;

  appendNode(n0, 1, 10);

  N *prev = n0;
  N *cur = n0->next;

  while (cur != NULL) {

    int v = prev->v;

    printf("%d\n", v);

    prev = cur;
    cur = cur->next;
  }

  return 0;
}
