#include <stdio.h>

int main (int argc, char *argv[]){

  /*
   * Fetch the inputs.
   */
  if (argc <= 2){
    fprintf(stderr, "USAGE: %s ITERATIONS INNER_ITERATIONS\n", argv[0], argv[1]);
    return 1;
  }
  auto iterations = atoll(argv[1]);
  auto innerIterations = atoll(argv[2]);

  /*
   * Allocate space.
   */
  int *ptr = (int *)calloc(iterations, sizeof(int));
  if (ptr == NULL){
    fprintf(stderr, "ERROR: %d integers couldn't be allocated\n", iterations);
    return 1;
  }
  
  /*
   * Hot code.
   */
  for (auto i=0; i < iterations; i++){
    for (auto j=0; j < innerIterations; j++){
      (*ptr) += (j * 42) ;
    }
    ptr++;
  }

  return 0;
}
