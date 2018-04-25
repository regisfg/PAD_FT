#include <stdio.h>
#include <sys/time.h>

static long num_steps = 1000000;
double step;
int main ()
{
  int i;
  double x, pi, sum = 0.0;

  struct timeval time;

  int sec[2], usec[2];
  double deltat;
  
  step = 1.0/(double) num_steps;
  
  // the initial clock time
  gettimeofday(&time, NULL);
  sec[0]  = (int)time.tv_sec;
  usec[0] = (int)time.tv_usec;

  for (i=0; i<num_steps; i++)
  {
     x = (i+0.5)*step;
     sum = sum + 4.0/(1.0+x*x);
  }
  
  // the final clock time
  gettimeofday(&time, NULL);
  sec[1]  = (int)time.tv_sec;
  usec[1] = (int)time.tv_usec;
  
  deltat = (((double)sec[1]) + ((double)usec[1])/1000000.0) - (((double)sec[0]) + ((double)usec[0])/1000000.0);
  printf("deltat = %.6f\n",deltat);
  
  pi = step * sum;
  
  printf("Valor de pi: %.30f\n", pi);
  return 0;
}
