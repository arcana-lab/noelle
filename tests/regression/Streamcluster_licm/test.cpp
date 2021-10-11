#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

struct Point {
  float weight; // the existence of this field triggers a bug inside SCAF and causes a mem dependece to be removed, which finally leads to a seg fault due to loading from uninitialized memory
  float *coord;
};

struct Points {
  Point *p;
};

void run(int num, int dim, Points *points) {
  float cost = 0.0;
  for (int i = 0; i < num; i++) {
    Point p0 = points->p[0];
    Point p1 = points->p[i];
    for (int j = 0; j < dim; j++) {
      cost += p0.coord[i] - p1.coord[j];
    }
  }

  fprintf(stdout, "%d\n", (int)cost);
}

int main(int argc, char **argv) {
  int num = atoi(argv[1]);
  int dim = atoi(argv[2]);

  Points points;
  points.p = (Point *)malloc(num * sizeof(Point));

  float *coordValues = (float *)malloc(dim * sizeof(float)); // all point have the same coord, this ensures a determined program output
  for (int i = 0; i < dim; i++) {
    coordValues[i] = lrand48() / (float)INT_MAX;
  }

  float *coords = (float *)malloc(num * dim * sizeof(float));
  for (int i = 0; i < num; i++) {
    memcpy(&coords[i * dim], coordValues, dim * sizeof(float));
    points.p[i].coord = &coords[i * dim];
    points.p[i].weight = 1.0;
  }

  run(num, dim, &points);
  return 0;
}
