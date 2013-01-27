#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include "util/util.h"

#ifndef ATYPE
#define ATYPE int
#endif

#ifndef SEP
#define SEP ";"
#endif

#define MINDEX(I,J) ((I)*n+(J))

static void genmatvec(ATYPE* matrix, ATYPE* vector, int m, int n);
static void benchmark(ATYPE matrix[], ATYPE vector[], ATYPE res[], int m, int n, double* dtime);
static void matmult(ATYPE mat[], ATYPE vec[], ATYPE* res, int m, int n);
static ATYPE* reference(int m, int n);

static void* xmalloc(int size);
static void print_matrix(const char* caption, ATYPE mat[], int m, int n);
static void print_vector(const char* caption, ATYPE vec[], int n);
static int vector_equal(ATYPE* a1, ATYPE* a2, int n);
static void print_perf(int m, int n, int t, double dtime);
static void print_perf_debug(int m, int n, int t, double dtime);
static void fail(const char* format, ...);

int main(int argc, char* argv[])
{
	int debug_flag;
	int m;
	int n;
	int t;

	if (parse_args(argc, argv, &debug_flag, "m size", &m, "n size", &n, "threads", &t, NULL, NULL) != 0)
	{
		fail("Error while parsing args.");
	}

	if ((n < 1) || (t < 1))
	{
		fail("Bad input");
	}

	omp_set_num_threads(t);

	ATYPE mat[m*n];
	ATYPE vec[n];
	ATYPE res[m]; // result vector

	genmatvec(mat, vec, m, n);

	if (debug_flag) print_matrix("mat", mat, m, n);
	if (debug_flag) print_vector("vec", vec, n);

	double dtime;
	benchmark(mat, vec, res, m, n, &dtime);

	if (debug_flag)
	{
		/* ========== OUTPUT =========== */

		print_vector("output", res, m);
		print_perf_debug(m, n, t, dtime);

		ATYPE* ref = reference(m, n);

		if (vector_equal(res, ref, m))
		{
			printf("SUCCESS\n");
		}
		else
		{
			printf("EPIC FAILURE\n");
		}

		free(ref);
	}
	else
	{
		print_perf(m, n, t, dtime);
	}
}


static void genmatvec(ATYPE* matrix, ATYPE* vector, int m, int n)
{
	for (int i = 0; i < m*n; i++)
	{
		matrix[i] = i % 5 + 1;
	}

	for (int i = 0; i < n; i++)
	{
		vector[i] = i % 7 + 3;
	}
}

static void benchmark(ATYPE* matrix, ATYPE* vector, ATYPE* res, int m, int n, double* dtime)
{
	double stime = omp_get_wtime();
	matmult(matrix, vector, res, m, n);
	double etime = omp_get_wtime();

	*dtime = etime - stime;
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

static ATYPE* reference(int m, int n)
{
	ATYPE* matrix = xmalloc(m * n * sizeof(ATYPE));
	ATYPE* vector = xmalloc(n * sizeof(ATYPE));
	ATYPE* result = xmalloc(m * sizeof(ATYPE));

	genmatvec(matrix, vector, m, n);

	for (int i = 0; i < m; i++) 
	{
		result[i] = 0;
		for (int j = 0; j < n; j++) 
		{
			result[i] += matrix[MINDEX(i,j)] * vector[j];
		}
	}

	free(matrix);
	free(vector);

	return result;
}


static void* xmalloc(int size)
{
	void* ret = malloc(size);
	
	if (NULL == ret)
	{
		fail("MALLOC ERROR: %s\n", strerror(errno));
	}
	
	return ret;
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

static int vector_equal(ATYPE* a1, ATYPE* a2, int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		if (a1[i] != a2[i]) return 0;
	}

	return 1;
}

static void print_perf(int m, int n, int t, double dtime)
{
	printf("%d%s%d%s%d%s%f\n", m, SEP, n, SEP, t, SEP, dtime);
}

static void print_perf_debug(int m, int n, int t, double dtime)
{
	printf("m size: %d\n", m);
	printf("n size: %d\n", n);
	printf("Threads: %d\n", t);
	printf("Time: %f\n", dtime);
}

static void fail(const char* format, ...)
{
	va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
	exit(1);
}
