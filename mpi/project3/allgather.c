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

static void genmatvec(ATYPE* submat, ATYPE* vector, int n, int size, int vsize, int rank);
static void allgather(ATYPE* vector, int vsize);
static void matmult(ATYPE* submat, ATYPE* vector, ATYPE* result, int m, int n);
static ATYPE* reference(int m, int n);

static void print_perf(int m, int n, int p, double dtime);
static void print_perf_debug(int m, int n, int p, double dtime);

static void* xmalloc(int size);
static void print_array(const char* caption, ATYPE* a, int n);
static int array_equal(ATYPE* a1, ATYPE* a2, int n);
static void fail(const char* format, ...);

/*
 * In this implementation of matrix-vector multiplication,
 * the matrix is split up between nodes by rows.
 */
int main(int argc, char* argv[])
{
	int debug_flag;
	int m;       // matrix total row count
	int n;       // vector input size
	int size;    // block size (local matrix row count)
	int vsize;   // vector block size (local vector size)
	int rank, p;
	int ret;     // return value, error indicator
	ATYPE* submat = NULL; // local submatrix
	ATYPE* vector = NULL; // input vector (gathered in-place)
	ATYPE* result = NULL; // local result vector chunk
	ATYPE* allresult = NULL; // global result vector
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

	size = m / p;
	vsize = n / p;

	if (p * size != m)
	{
		fail("ERROR: %d matrix rows cannot be exactly fit to %d nodes.\n", m, p);
	}

	if (p * vsize != n)
	{
		fail("ERROR: %d vector rows cannot be exactly fit to %d nodes.\n", n, p);
	}

	if (0 == rank)
	{
		time_start = MPI_Wtime();
	}

	/* Calculations */

	submat = xmalloc(size * n * sizeof(ATYPE));
	vector = xmalloc(n * sizeof(ATYPE));
	genmatvec(submat, vector, n, size, vsize, rank);
	allgather(vector, vsize);
	result = xmalloc(size * sizeof(ATYPE));
	matmult(submat, vector, result, size, n);

	/* Measure performance */

	if (0 == rank)
	{
		time_end = MPI_Wtime();

		if (debug_flag)
		{
			print_perf_debug(m, n, p, time_end - time_start);
		}
		else
		{
			print_perf(m, n, p, time_end - time_start);
		}
	}

	/* Gather */

	if (debug_flag)
	{
		if (0 == rank)
		{
			allresult = xmalloc(m * sizeof(ATYPE));
			ret = MPI_Gather(result, size, ATYPE_MPI, allresult, size, ATYPE_MPI, 0, MPI_COMM_WORLD);
			print_array("Result", allresult, m);

			ATYPE* ref = reference(m, n);
			print_array("Reference", ref, m);
			if (array_equal(allresult, ref, m) != 0)
			{
				printf("SUCCESS\n");
			}
			else
			{
				printf("EPIC FAILURE\n");
			}
			free(ref);
			free(allresult);
		}
		else
		{
			ret = MPI_Gather(result, size, ATYPE_MPI, NULL, size, ATYPE_MPI, 0, MPI_COMM_WORLD);	
		}
	}

	MPI_Finalize();

	free(submat);
	free(vector);
	free(result);

	return 0;
}

static void genmatvec(ATYPE* submat, ATYPE* vector, int n, int size, int vsize, int rank)
{
	int mstart = rank * size * n;
	int vstart = rank * vsize;

	for (int i = 0; i < size*n; i++)
	{
		submat[i] = (i + mstart) % 5 + 1;
	}

	for (int i = vstart; i < vstart+vsize; i++)
	{
		vector[i] = i % 7 + 3;
	}
}

static void allgather(ATYPE* vector, int vsize)
{
	int ret = MPI_Allgather(MPI_IN_PLACE, vsize, ATYPE_MPI, vector, vsize, ATYPE_MPI, MPI_COMM_WORLD);
}

static void matmult(ATYPE* submat, ATYPE* vector, ATYPE* result, int m, int n)
{
	for (int i = 0; i < m; i++)
	{
		result[i] = 0;
		for (int j = 0; j < n; j++)
		{
			result[i] += submat[i*n+j] * vector[j];
		}
	}
}

static ATYPE* reference(int m, int n)
{
	ATYPE* mat = xmalloc(m * n * sizeof(ATYPE));
	ATYPE* vec = xmalloc(n * sizeof(ATYPE));
	ATYPE* res = xmalloc(m * sizeof(ATYPE));
	genmatvec(mat, vec, n, m, n, 0);
	matmult(mat, vec, res, m, n);
	free(mat);
	free(vec);
	return res;
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
