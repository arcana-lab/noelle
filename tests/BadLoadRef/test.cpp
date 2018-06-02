#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
void copycenters(Points *points, Points* centers, long* centerIDs, long offset)
{
  long i;
  long k;

  bool *is_a_median = (bool *) calloc(points->num, sizeof(bool));

  for ( i = 0; i < points->num; i++ ) {
    is_a_median[points->p[i].assign] = 1;
  }

  k=centers->num;

  for ( i = 0; i < points->num; i++ ) {
    if ( is_a_median[i] ) {
      memcpy( centers->p[k].coord, points->p[i].coord, points->dim * sizeof(float));
      centers->p[k].weight = points->p[i].weight;
      centerIDs[k] = i + offset;
      k++;
    }
  }

  centers->num = k;

  free(is_a_median);
}
*/

struct Point {
  float val;
  int index;
  Point(float v, int i) : val{v}, index{i} {};
};

struct Points {
  Point *p;
}; 

int main (){
  Points points;
  points.p = (Point *) malloc(3 * sizeof(Point));

  for (int i = 0; i < 3; ++i) {
    points.p[i].index = i;
    points.p[i].val = 11 + i * 7;
  }

  printf("Pointer value: %f\n", points.p[2].val);
  free(points.p);
  return 0;
}

