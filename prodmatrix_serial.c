#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

void zeros(int **m);
void read_matrix(int **m, char *fname);
void matrix_product();
void write_matrix(int **m, char *fname);

#define _MSG_CMD_LINE0    0
#define _MSG_CMD_LINE1    1
#define _MSG_MTX_CANCALC  2
#define _MSG_MTX_SUCSSLD  3
#define _MSG_ERR_GENERR   4
#define _MSG_ERR_INCNPAR  5
#define _MSG_ERR_FNOTFND  6
#define _MSG_ERR_MTXLTN2  7
#define _MSG_ERR_MTXDIM   8
#define _MSG_ERR_CNTCALC  9
#define _MSG_ERR_MTXSIZE 10
#define _MSG_ERR_NTHSIZE 11
#define _MSG_ERR_WRTFILE 12
#define _MSG_WRN_GENWARN 13
#define _MSG_WRN_MTXGTN2 14

// global variables

char *msg[15] = { \
	"<matrix1> <matrix2> <matrix_output> <size>", \
	"where: \nmatrix1, matrix2, matrix_output: the filenames for each matrix\nsize: the number of lines and columns (NxN), an eger between 1 and 1000\nnumber_of_threads: an eger between 1 and 8\n", \
	"The product m1 x m2 can be calculated!", \
	"Matrix sucessfully loaded!", \
	"ERROR:", \
	"Incorrect number of parameters", \
	"File not found", \
	"Error reading matrix", \
	"Both matrices needs to be square matrices and\nof the same dimensions dim(m1) = dim(m2) = NxN", \
	"Can't calculate the product!\nPossible causes:\n1: matrices m1 and m2 are not of the same size\n2: at least 1 matrix is not a square matrix", \
	"Number of lines and columns needs to be an eger between 1 and 1000", \
	"Number of threads neeeds to be an eger between 1 and 8", 
	"Could not open file for writing. Filename: ", \
	"WARNING:", \
	"The number of elements (numelems) of the matrix file\nis greater than N^2: numelems >"
};

// the matrices!
int **m1, **m2, **prod;

// the size of matrices (NxN), defined by command line parameter
long N;

int main(int argc, char *argv[]){
	int i = 0;
	int error = 0;
	
	struct timeval time;
	
	int sec[2], usec[2];
	double deltat;
	
	switch(argc){
		case 5:
			// set matrix size and number by command line parameter
			sscanf(argv[4], "%ld", &N);
			
			if (N<1 || N>1000){
				error = _MSG_ERR_MTXSIZE;
				printf("%s %s\n", msg[_MSG_ERR_GENERR], msg[_MSG_ERR_MTXSIZE]);
			}
			
			if (error > 0) exit(error);
			
			break;
		default:
			printf("%s %s\n\nUSAGE: %s %s\n%s\n", msg[_MSG_ERR_GENERR], msg[_MSG_ERR_INCNPAR], &argv[0][2], msg[_MSG_CMD_LINE0], msg[_MSG_CMD_LINE1]);
			printf("%s\n", msg[_MSG_ERR_MTXDIM]);
			exit(_MSG_ERR_INCNPAR);
			break;
	}
	
	// alocate the matrices
	// ***********************************************************************
	// credits: https://www.geeksforgeeks.org/dynamically-allocate-2d-array-c/
	m1    = (int **)malloc(sizeof(int *) * N);
	m2    = (int **)malloc(sizeof(int *) * N);
	prod  = (int **)malloc(sizeof(int *) * N);
	
	for(i = 0; i < N; i++){
		m1[i]   = (int *)malloc(N * sizeof(int));
		m2[i]   = (int *)malloc(N * sizeof(int));
		prod[i] = (int *)malloc(N * sizeof(int));
	}
	
	// initialize the matrices
	zeros(m1);
	zeros(m2);
	zeros(prod);
	
	// read matrix m1 and m2 from text files
	read_matrix(m1, argv[1]);
	read_matrix(m2, argv[2]);
	
	// the initial clock time
	gettimeofday(&time, NULL);
	sec[0]  = (int)time.tv_sec;
	usec[0] = (int)time.tv_usec;
	
	// calculate the product
	matrix_product(m1, m2, prod);
	write_matrix(prod, argv[3]);
	
	// the final clock time
	gettimeofday(&time, NULL);
	sec[1]  = (int)time.tv_sec;
	usec[1] = (int)time.tv_usec;
	
	deltat = (((double)sec[1]) + ((double)usec[1])/1000000.0) - (((double)sec[0]) + ((double)usec[0])/1000000.0);
	printf("deltat = %.6f\n",deltat);
}

void zeros(int **m){
	int i, j;
	for(i=0; i<N; i++){
		for(j=0; j<N; j++){
			m[i][j] = 0;
		}
	}
}

void read_matrix(int **m, char *fname){
	int i=0, j=0;
	int result;
	long count_elems = 0;
	
	FILE *pFile = NULL;
	
	pFile = fopen(fname, "r");
	if (pFile != NULL){
		for(i=0; i<N; i++){
			for(j=0; j<N; j++){
				result = fscanf(pFile, "%d", &m[i][j]);
				count_elems += result;
			}
		}
		// verify if there is at least 1 more element o matrix file
		result = fscanf(pFile, "%d", &i);
		if (result == 1){
			count_elems += result;
		}
		
		fclose(pFile);
		
		if (count_elems < N*N){
			printf("%s: \n", msg[_MSG_ERR_MTXLTN2]);
			exit(_MSG_ERR_MTXLTN2);
		}
		else if (count_elems > N*N){
			printf("%s file %s: %s %d\n", msg[_MSG_WRN_GENWARN], fname, msg[_MSG_WRN_MTXGTN2], N*N);
		}
//		else{
//			printf("%s\n", msg[_MSG_MTX_SUCSSLD]);
//		}
		
	}
	else{
		printf("%s %s\n", msg[_MSG_ERR_GENERR], msg[_MSG_ERR_FNOTFND]);
		exit(_MSG_ERR_FNOTFND);
	}
}

void matrix_product(){
	// parallelize by multiplying N/NTH lines
	// of the first matrix by the second matrix
	
 	int i, j, k;
	
	for (i=0; i<N; i++){
		for (j=0; j<N; j++){
			prod[i][j] = 0;
			for (k=0; k<N; k++){
				prod[i][j] += m1[i][k]*m2[k][j];
			}
		}
	}
}

void write_matrix(int **m, char *fname){
	int i, j;
	
	FILE *pFile = NULL;
	
	pFile = fopen(fname, "w");
	
	if (pFile != NULL){
		for(i=0; i<N; i++){
			for(j=0; j<N; j++){
				fprintf(pFile, "\t%hd", m[i][j]);
			}
		}
		
		fclose(pFile);
	}
	else{
		printf("%s: %s %s\n", msg[_MSG_ERR_GENERR], _MSG_ERR_WRTFILE, fname);
		exit(_MSG_ERR_WRTFILE);
	}
}
