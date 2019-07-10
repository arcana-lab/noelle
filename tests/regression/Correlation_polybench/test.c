#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

  static
void init_array (int m,
    int n,
    double *float_n,
    double data[m][n])
{
  int i, j;

  *float_n = 1.2;

  for (i = 0; i < m; i++)
    for (j = 0; j < n; j++)
      data[i][j] = ((double) i*j) / 1000;
}

  static
void print_array(int m,
    double symmat[m][m])

{
  int i, j;

  for (i = 0; i < m; i++)
    for (j = 0; j < m; j++) {
      fprintf (stderr, "%0.2lf ", symmat[i][j]);
    }
  fprintf (stderr, "\n");
}

  static
void kernel_correlation(int m, int n,
    double float_n,
    double data[m][n],
    double symmat[m][m])
{
  int i, j, j1, j2;

  double eps = 0.1f;

  for (j1 = 0; j1 < m-1; j1++)
  {
    symmat[j1][j1] = 1.0;
    for (j2 = j1+1; j2 < m; j2++)
    {
      symmat[j1][j2] = 0.0;
      for (i = 0; i < n; i++)
        symmat[j1][j2] += (data[i][j1] * data[i][j2]);
      symmat[j2][j1] = symmat[j1][j2];
    }
  }
  symmat[m-1][m-1] = 1.0;
}


int main(int argc, char** argv)
{

  int n = atoi(argv[1]);
  int m = atoi(argv[1]);

  double float_n;
  double (*data)[m][n]; data = (double(*)[m][n])malloc((m) * (n) * sizeof(double));;
  double (*symmat)[m][m]; symmat = (double(*)[m][m])malloc((m) * (m) * sizeof(double));;

  init_array (m, n, &float_n, *data);

  kernel_correlation (m, n, float_n,
      *data,
      *symmat);

  print_array(m, *symmat);

  free((void*)data);;
  free((void*)symmat);;

  return 0;
}
