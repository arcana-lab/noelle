#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
  long assign;
  long cost;
} Point;

typedef struct {
  int num;
  Point *p;
} Points;

void myF (Points *points) {
  long i, j;
  Point temp;

  for (i=0;i < points->num - 1; i++) {
    j=(lrand48()%(points->num - i)) + i;
    temp = points->p[i];
    points->p[i] = points->p[j];
    points->p[j] = temp;
  }

  return ;
}

int main (int argc, char *argv[]){
  Points points;

  /*
   * Check the inputs.
   */
  if (argc < 2){
    fprintf(stderr, "USAGE: %s POINTS\n", argv[0]);
    return -1;
  }

  /*
   * Allocate the memory.
   */
  points.num = (atoll(argv[1]) + 1) * 10;
  points.p = (Point *) malloc (sizeof(Point) * points.num);
  for (auto i=0; i < points.num; i++){
    points.p[i].cost = i;
    points.p[i].assign = i * 5;
  }

  /*
   * Work
   */
  myF(&points);

  long long int cost = 0;
  long long int assign = 0;
  for (auto i=0; i < points.num; i++){
    cost += points.p[i].cost;
    assign += points.p[i].assign;
  }
  printf("%lld %lld\n", cost, assign);

  return 0;
}
