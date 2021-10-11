#include <stdio.h>
#include <stdlib.h>

struct Point {
  float weight; // the existence of this field caused a mem dependence removed by SCAF and leads to a seg fault due to loading from uninitialized memory
  float *coord;
};

struct Points {
  Point *p;
};

void run(int num, int dim, Points *points) {
  float cost = 0.0;
  for (int i = 0; i < num; i++) {
    Point p = points->p[i];
    for (int j = 0; j < dim; j++) {
      cost += p.coord[j];
    }
  }

  fprintf(stderr, "%f\n", cost);
}

int main(int argc, char **argv) {
  int num = atoi(argv[1]);
  int dim = atoi(argv[2]);

  Points points;
  points.p = (Point *)malloc(num * sizeof(Point));

  float *coords = (float *)malloc(num * dim * sizeof(float));
  for (int i = 0; i < num; i++) {
    points.p[i].coord = &coords[i * dim];
    points.p[i].weight = 1.0;
  }

  run(num, dim, &points);
  return 0;
}
