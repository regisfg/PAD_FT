#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

static long num_steps = 1000000;
double step;

// stores the value of each slice of the calculus
double *part_sums;

// stores limits for each thread (the first and last positions of the calculus for each thread)
long **a_limits;

void *calcPi_slice(void *thid);

int main (int argc, char *argv[])
{
	int ret;
	
	struct timeval time;
	
	int sec[2], usec[2];
	double deltat;
	
	long i, NTH = 0; // NTH: number of threads
	double pi = 0.0;
	
	step = 1.0/(double) num_steps;
	
	// check if the command line argument is ok
	switch(argc){
		case 2:
			// read the number of threads by command line parameter and the stores it ---> NTH
			sscanf(argv[1], "%ld", &NTH);
			printf("NTH=%d\n", NTH);
			
			if (NTH<1 || NTH>8){
				printf("The number of threads needs to be an integer between 1 and 8\n");
				exit(2);
			}
			break;
			
		default:
			printf("USAGE: %s <number_of_threads>\n", &argv[0][2]);
			exit(1);
			break;
	}
	
	// struct to manage threads
	pthread_t th[NTH];
	
	// partial sums of Pi
	part_sums = malloc(NTH*sizeof(double));  // will be initialized by each thread!
	
	// alocate a_limits
	// ***********************************************************************
	// adapted from: https://www.geeksforgeeks.org/dynamically-allocate-2d-array-c/
	a_limits = (long **)malloc(sizeof(long *) * NTH);
	
	for(i = 0; i < NTH; i++){
		a_limits[i] = (long *)malloc(2 * sizeof(long));
	}
	
	// the initial clock time
	gettimeofday(&time, NULL);
	sec[0]  = (int)time.tv_sec;
	usec[0] = (int)time.tv_usec;
	
	// initialize a_limits
	for(i=0; i<NTH; i++){
		a_limits[i][0] = (num_steps/NTH)*i;
		a_limits[i][1] = (num_steps/NTH)*(1+i) - 1;
	}
	// avoid losing the last elements if N is not divisible by NTH
	a_limits[NTH-1][1] = num_steps-1;
	
	// create each thread
	for(i=0; i<NTH; i++){
		ret = pthread_create(&th[i], NULL, calcPi_slice, (void *) i);
	}
	
	for(i=0; i<NTH; i++){
		pthread_join(th[i], NULL);
		pi += part_sums[i];
	}
	
	// the final clock time
	gettimeofday(&time, NULL);
	sec[1]  = (int)time.tv_sec;
	usec[1] = (int)time.tv_usec;
	
	deltat = (((double)sec[1]) + ((double)usec[1])/1000000.0) - (((double)sec[0]) + ((double)usec[0])/1000000.0);
	printf("deltat = %.6f\n",deltat);
	
	printf("Valor de pi: %.30f\n", pi);
	
	return 0;
}

void *calcPi_slice(void *thid){
 	long i, f, l;	// f, l: the first (f) and the last (l) elements for each 'slice' of the calculus
	long mythid = (long)thid;
	
	double x, sum  = 0.0;
	
	// limits for each slice
	f = a_limits[mythid][0];
	l = a_limits[mythid][1];
	
	for (i=f; i<=l; i++)
	{
		x = (i+0.5)*step;
		sum = sum + 4.0/(1.0+x*x);
	}
	
	part_sums[mythid] = sum*step;
}
