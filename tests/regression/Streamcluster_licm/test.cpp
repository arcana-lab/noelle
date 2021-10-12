#include <stdio.h>
#include <stdlib.h>

struct Point {
  float weight; // the existence of this field triggers a bug inside SCAF and causes a mem dependece to be removed, which finally leads to a seg fault due to loading from uninitialized memory
  float *coord;
};

struct Points {
  Point *p;
};

int main(int argc, char **argv) {
  int value = atoi(argv[1]);
  
  Points points;
  points.p = (Point *)malloc(sizeof(Point));
  points.p[0].coord = (float *)malloc(sizeof(float));
  points.p[0].coord[0] = value;  
  
  float cost = 0.0;
  for (int i = 0; i < 1; i++) {
    Point p = points.p[i];
    cost += p.coord[0];
  }

  fprintf(stdout, "%d\n", (int)cost);
  return 0;
}
