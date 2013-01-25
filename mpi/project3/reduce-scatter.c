#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <errno.h>
#include <stdarg.h>
#include "util/util.h"

#ifndef ATYPE
#define ATYPE int
#define ATYPE_MPI MPI_INT
#endif

#ifndef SEP
#define SEP ";"
#endif

static void genmatvec(ATYPE* submat, ATYPE* vector, int m, int n, int size, int rank);
static void partmult(ATYPE* submat, ATYPE* vector, ATYPE* result, int m, int size);
static void allgather(ATYPE* block, int n);

static void print_perf(int m, int n, int p, double dtime);
static void print_perf_debug(int m, int n, int p, double dtime);

static void* xmalloc(int size);
static void print_array(const char* caption, ATYPE* a, int n);
static int array_equal(ATYPE* a1, ATYPE* a2, int n);
static void fail(const char* format, ...);

/*
 * In this implementation of matrix-vector multiplication,
 * the matrix is split up between nodes by columns.
 */
int main(int argc, char* argv[])
{
	int debug_flag;
	int m;       // matrix total row count
	int n;       // vector input size
	int size;    // block size
	int rsize;   // result block size
	int rank, p;
	int ret;     // return value, error indicator
	ATYPE* submat = NULL;     // local submatrix
	ATYPE* vector = NULL;     // input vector
	ATYPE* result = NULL;     // local result
	ATYPE* allresult = NULL;  // global result vector
	double time_start, time_end; // unused in nodes > 0

	if (parse_args(argc, argv, &debug_flag, "result size", &m, "vector size", &n, NULL, NULL, NULL, NULL) != 0)
	{
		fail("Error parsing command args.\n");
	}

	if ((m < 1) || (n < 1))
	{
		fail("Bad input?.\n");
	}

	MPI_Init (&argc, &argv);

	// get rank and size from communicator
	MPI_Comm_size (MPI_COMM_WORLD, &p);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);

	size = n / p;
	rsize = m / p;

	if (p * size != n)
	{
		fail("ERROR: %d input vector elements cannot be exactly fit to %d nodes.\n", n, p);
	}

	if (p * rsize != m)
	{
		fail("ERROR: %d result vector elements cannot be exactly fit to %d nodes.\n", m, p);
	}

	if (0 == rank)
	{
		time_start = MPI_Wtime();
	}

	/* Calculations */

	submat = xmalloc(size * m * sizeof(ATYPE));
	vector = xmalloc(size * sizeof(ATYPE));
	result = xmalloc(m * sizeof(ATYPE));
	genmatvec(submat, vector, m, n, size, rank);
	partmult(submat, vector, result, m, size);

	/* Scatter-reduce */
	int* counts = xmalloc(p * sizeof(int));
	for (int i = 0; i < p; i++) counts[i] = rsize;
	ret = MPI_Reduce_scatter(MPI_IN_PLACE, result, counts, ATYPE_MPI, MPI_SUM, MPI_COMM_WORLD);

	/* Gather */

	if (0 == rank)
	{
		allresult = xmalloc(m * sizeof(ATYPE));
		ret = MPI_Gather(result, rsize, ATYPE_MPI, allresult, rsize, ATYPE_MPI, 0, MPI_COMM_WORLD);
	}
	else
	{
		ret = MPI_Gather(result, rsize, ATYPE_MPI, NULL, rsize, ATYPE_MPI, 0, MPI_COMM_WORLD);	
	}

	/* Measure performance */

	if (0 == rank)
	{
		time_end = MPI_Wtime();

		if (debug_flag)
		{
			print_array("Result", allresult, m);
			print_perf_debug(m, n, p, time_end - time_start);
		}
		else
		{
			print_perf(m, n, p, time_end - time_start);
		}

		free(allresult);
	}

	MPI_Finalize();

	free(counts);
	free(submat);
	free(vector);
	free(result);

	return 0;
}

static void genmatvec(ATYPE* submat, ATYPE* vector, int m, int n, int size, int rank)
{
	for (int i = 0; i < m; i++)
	for (int j = 0; j < size; j++)
	{
		int mstart = i * n + rank * size;
		submat[i*size+j] = (j + mstart) % 5 + 1;
	}

	int vstart = rank * size;

	for (int i = 0; i < size; i++)
	{
		vector[i] = (i + vstart) % 7 + 3;
	}
}

static void partmult(ATYPE* submat, ATYPE* vector, ATYPE* result, int m, int size)
{
	for (int i = 0; i < m; i++)
	{
		result[i] = 0;
		for (int j = 0; j < size; j++)
		{
			result[i] += submat[i*size+j] * vector[j];
		}
	}
}

static void allgather(ATYPE* vector, int vsize)
{
	int ret = MPI_Allgather(MPI_IN_PLACE, vsize, ATYPE_MPI, vector, vsize, ATYPE_MPI, MPI_COMM_WORLD);
}


static void print_perf(int m, int n, int p, double dtime)
{
	printf("%d%s%d%s%d%s%f\n", m, SEP, n, SEP, p, SEP, dtime);
}

static void print_perf_debug(int m, int n, int p, double dtime)
{
	printf("Nodes: %d\n", p);
	printf("Result size: %d\n", m);
	printf("Input vector size: %d\n", n);
	printf("Time: %f\n", dtime);
}


static void* xmalloc(int size)
{
	void* ret = malloc(size);
	if (NULL == ret)
	{
		fail("MALLOC ERROR: %s\n", strerror(errno));
	}
	else return ret;
}

static void print_array(const char* caption, ATYPE* a, int n)
{
	if (NULL != caption) printf("%s:\n", caption);

	for (int i = 0; i < n; i++)
	{
		printf("%d  ", a[i]);
	}
	printf("\n");
}

static int array_equal(ATYPE* a1, ATYPE* a2, int n)
{
	for (int i = 0; i < n; i++)
	{
		if (a1[i] != a2[i]) return 0;
	}

	return 1;
}

static void fail(const char* format, ...)
{
	va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
	exit(1);
}
