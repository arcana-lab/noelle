#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

struct _NODE {
  int i1;
  int i2;
  int i3;
};
typedef struct _NODE NODE;

int main (int argc, char *argv[]){

  auto NUM_NODES = 10 * argc;
  NODE nodes[NUM_NODES];

  NODE *nodePtr1 = (NODE *)&nodes;
  for (auto i = 0; i < NUM_NODES; ++i) {
    nodePtr1->i1 = i;
    nodePtr1->i2 = i + argc;
    nodePtr1->i3 = 1;
    nodePtr1++;
  }

  NODE *nodePtr2 = (NODE *)&nodes;
  for (auto i = 0; i < NUM_NODES; ++i) {
    printf("%d, %d, %d\n", nodePtr2->i1, nodePtr2->i2, nodePtr2->i3);
    nodePtr2++;
  }

  return 0;
}


