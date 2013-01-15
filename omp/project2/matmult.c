#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include "util/args.h"

#ifndef ATYPE
#define ATYPE int
#endif

#define MINDEX(I,J) ((I)*n+(J))

static void benchmark(ATYPE matrix[], ATYPE vector[], ATYPE res[], int m, int n);
static void matmult();
static void print_matrix(const char* caption, ATYPE mat[], int m, int n);
static void print_vector(const char* caption, ATYPE vec[], int n);
static void print_perf(int m, int n, int t);

int main(int argc, char* argv[])
{
	int i;
	int debug_flag;
	int m;
	int n;
	int t;

	if (parse_args(argc, argv, &debug_flag, "m size", &m, "n size", &n, "threads", &t, NULL, NULL) != 0)
	{
		exit(1);
	}

	if ((n < 1) || (t < 1))
	{
		fprintf(stderr, "%s\n", "bad input?");
		exit(1);
	}

	omp_set_num_threads(t);

	ATYPE mat[m*n];
	ATYPE vec[n];
	ATYPE res[m]; // result vector

	for (i = 0; i < m*n; i++) 
	{
		mat[i] = (i % 17) + 2;
	}

	for (i = 0; i < n; i++) 
	{
		vec[i] = (i % 3)*3 -4;
	}

	if (debug_flag) print_matrix("mat", mat, m, n);
	if (debug_flag) print_vector("vec", vec, n);
	benchmark(mat, vec, res, m, n);

	if (debug_flag) print_vector("res", res, m);
	print_perf(m, n, t);
}

static void benchmark(ATYPE matrix[], ATYPE vector[], ATYPE res[], int m, int n)
{
	double stime = omp_get_wtime();
	matmult(matrix, vector, res, m, n);
	double etime = omp_get_wtime();

	printf("Time: %f\n", etime - stime);
}

static void matmult(ATYPE mat[], ATYPE vec[], ATYPE* res, int m, int n)
{
	int i, j;

	#pragma omp parallel for schedule(static,1)
	for (i = 0; i < m; i++) 
	{
		res[i] = 0;
		for (j = 0; j < n; j++) 
		{
			res[i] += mat[MINDEX(i,j)] * vec[j];
		}
	}
}

static void print_matrix(const char* caption, ATYPE mat[], int m, int n)
{
	if (NULL != caption) printf("%s:\n", caption);

	for (int i = 0; i < m; i++)
	{
		printf("[");
		for (int j = 0; j < n; j++)
		{
			printf("%d,", mat[MINDEX(i,j)]);
		}
		printf("]\n");
	}
}

static void print_vector(const char* caption, ATYPE vec[], int n)
{
	if (NULL != caption) printf("%s:\n", caption);

	printf("[");
	for (int i = 0; i < n; i++) 
	{
		printf("%d,", vec[i]);
	}
	printf("]\n");
}

static void print_perf(int m, int n, int t)
{
	printf("m size: %d\n", m);
	printf("n size: %d\n", n);
	printf("Threads: %d\n", t);
}
