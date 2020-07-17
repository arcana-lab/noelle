extern "C" int nk_vc_printf (...);
extern "C" long atol (...);

long long int *array;

long long int computeSum (long long int *a, long long int iters, long long unsigned int inner_iters){
  long long int s =0;

  for (auto i=0; i < iters; ++i){

    double tmp = 43290.32435F;
    for (auto j=0; j < inner_iters; j++){
      tmp = tmp - 2;
    }

    s += tmp;
  }

  return s;
}

int main (int argc, char *argv[]){

  /*
   * Check the inputs.
   */
  auto iterations = atol(argv[1]);
  auto inner_iterations = atol(argv[2]);

  auto s = computeSum(array, iterations, inner_iterations);
  nk_vc_printf("%lld\n", s);

  return 0;
}
