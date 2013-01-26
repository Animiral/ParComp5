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

static ATYPE* genblock(int rank, int n, int p, int* size);
static void seq_sum(ATYPE* block, int n);
static void dist_sum(int rank, int p, ATYPE b, ATYPE* prefix);
static void add_prefix(ATYPE* block, int n, int prefix);
static ATYPE* reference(int n);

static void print_perf(int n, int p, double dtime);
static void print_perf_debug(int n, int p, double dtime);

static void* xmalloc(int size);
static void print_array(const char* caption, ATYPE* a, int n);
static int array_equal(ATYPE* a1, ATYPE* a2, int n);
static void fail(const char* format, ...);

int main(int argc, char* argv[])
{
	int debug_flag;
	int n;       // input size
	int size;    // block size
	ATYPE prefix;  // partial sum from lower ranked nodes
	int rank, p;
	ATYPE* block = NULL;
	ATYPE* result = NULL;
	double time_start, time_end; // unused in nodes > 0

	if (parse_args(argc, argv, &debug_flag, "input size", &n, NULL, NULL, NULL, NULL, NULL, NULL) != 0)
	{
		fail("Error parsing command args.\n");
	}

	if (n < 1)
	{
		fail("Bad input?.\n");
	}

	MPI_Init (&argc, &argv);

	// get rank and size from communicator
	MPI_Comm_size (MPI_COMM_WORLD, &p);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);

	if (0 == rank)
	{
		time_start = MPI_Wtime();
	}

	/* Calculations */

	block = genblock(rank, n, p, &size);
	seq_sum(block, size);
	dist_sum(rank, p, block[size-1], &prefix);
	add_prefix(block, size, prefix);

	/* Measure performance */

	if (0 == rank)
	{
		time_end = MPI_Wtime();

		if (debug_flag)
		{
			print_perf_debug(n, p, time_end - time_start);
		}
		else
		{
			print_perf(n, p, time_end - time_start);
		}
	}

	/* Gather */

	if (debug_flag)
	{
		if (0 == rank)
		{
			// TODO: use MPI gather operations
			result = xmalloc(n * sizeof(ATYPE));
			int c;

			for (c = 0; c < size; c++)
			{
				result[c] = block[c];
			}

			for (int peer = 1; peer < p; peer++)
			{
				int bsize = n / p + ((peer < n % p) ? 1 : 0);
				MPI_Recv(block, bsize, ATYPE_MPI, peer, peer, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				for (int i = 0; i < bsize; i++)
				{
					result[c+i] = block[i];
				}
				c += bsize;
			}

			/* ========== OUTPUT =========== */

			print_array("Result", result, n);
			ATYPE* ref = reference(n);
			if (array_equal(result, ref, n))
			{
				printf("SUCCESS\n");
			}
			else
			{
				printf("EPIC FAILURE\n");
			}
			free(ref);
			free(result);
		}
		else
		{
			MPI_Send(block, size, ATYPE_MPI, 0, rank, MPI_COMM_WORLD);
		}
	}

	MPI_Finalize();

	free(block);

	return 0;
}

static ATYPE* genblock(int rank, int n, int p, int* size)
{
	*size = n / p;
	int rem = n % p;
	if (rank < rem)
	{
		(*size)++;
	}

	ATYPE* block = xmalloc((*size) * sizeof(ATYPE));

	for (int i = 0; i < *size; i++)
	{
		int global_i = n * rank / p + (((n*rank) % p) ? 1 : 0) + i;
		// block[i] = (global_i % 3) == 0;
		block[i] = global_i + 1;
	}

	return block;
}

static void seq_sum(ATYPE* block, int n)
{
	for (int i = 1; i < n; i++)
	{
		block[i] += block[i-1];
	}
}

static void dist_sum(int rank, int p, ATYPE b, ATYPE* prefix)
{
	ATYPE buf;
	*prefix = 0;

	for (int k = 1; k < p; k<<=1) 
	{
		int rpeer = rank - k;
		int speer = rank + k;

		if (speer >= p)
		{
			speer = MPI_PROC_NULL;	
		}

		if (rpeer < 0)
		{
			buf = 0;
			rpeer = MPI_PROC_NULL;
		}

		int ret = MPI_Sendrecv(&b, 1, ATYPE_MPI, speer, k, &buf, 1, ATYPE_MPI, rpeer, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		
		*prefix += buf;
		b += buf;
	}
}

static void add_prefix(ATYPE* block, int n, int prefix)
{
	for (int i = 0; i < n; i++)
	{
		block[i] += prefix;
	}
}

static ATYPE* reference(int n)
{
	int s;
	ATYPE* a = genblock(0, n, 1, &s);

	if (s != n)
	{
		fail("genblock returned wrong size (%d) for reference (n=%d).", s, n);
	}

	for (int i = 1; i < n; i++)
	{
		a[i] += a[i-1];
	}

	return a;
}


static void print_perf(int n, int p, double dtime)
{
	printf("%d%s%d%s%f\n", p, SEP, n, SEP, dtime);
}

static void print_perf_debug(int n, int p, double dtime)
{
	printf("Nodes: %d\n", p);
	printf("Total input size: %d\n", n);
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
