extern "C" int nk_vc_printf (...);
extern "C" long atol (...);

long long int *array;

long int computeSum (long long int *a, long long int iters, long long unsigned int inner_iters){
  long int s =0;

  for (auto i=0; i < iters; ++i){

    long int tmp = 43290;
    for (auto j=0; j < inner_iters; j++){
      if (tmp < 100) {
        tmp = tmp - 2;
      } else {
        tmp = tmp - 5 * 2;
      }
    }

    s += tmp;
  }
  nk_vc_printf("COOL0 %ld\n", s);

  return s;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  nk_vc_printf("BEGIN\n");
  auto iterations = atol(argv[1]);
  nk_vc_printf("  ITERATIONS %d\n", iterations);
  auto inner_iterations = atol(argv[2]);
  nk_vc_printf("  INNER %d\n", inner_iterations);

  long int s = computeSum(array, iterations, inner_iterations);
  nk_vc_printf("COOL1 %ld\n", s);
  nk_vc_printf("COOL2 %ld\n", 42);

  return s;
}
