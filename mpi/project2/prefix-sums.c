#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <errno.h>
#include <stdarg.h>
#include "util/args.h"

#ifndef ATYPE
#define ATYPE int
#define ATYPE_MPI MPI_INT
#endif

#ifndef SEP
#define SEP ";"
#endif

static ATYPE* genblock(int rank, int n, int p, int* size);
static void seq_sum(ATYPE* block, int n);
static void dist_sum(int rank, int p, int b, int* prefix);
static void add_prefix(ATYPE* block, int n, int prefix);

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
	int prefix;  // partial sum from lower ranked nodes
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

	/* Gather */

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
	}
	else
	{
		MPI_Send(block, size, ATYPE_MPI, 0, rank, MPI_COMM_WORLD);
	}

	/* Measure performance */

	if (0 == rank)
	{
		time_end = MPI_Wtime();

		if (debug_flag)
		{
			print_array("Result", result, n);
			print_perf_debug(n, p, time_end - time_start);
		}
		else
		{
			print_perf(n, p, time_end - time_start);
		}

		free(result);
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
		int global_i = n * rank / p + ((n % p) ? 1 : 0) + i;
		block[i] = (global_i % 3) == 0;
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

static void send_b(int peer, int p, int b)
{
	int ret;
	if (peer < p)
	{
		ret = MPI_Send(&b, 1, ATYPE_MPI, peer, 0, MPI_COMM_WORLD);
	}
}

static ATYPE recv_b(int peer)
{
	int ret;
	ATYPE buf;
	if (peer >= 0)
	{
		ret = MPI_Recv(&buf, 1, ATYPE_MPI, peer, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
		return buf;
	}
	else
	{
		return (ATYPE)0;
	}
}

static void dist_sum(int rank, int p, int b, int* prefix)
{
	ATYPE buf;
	*prefix = 0;

	for (int k = 1; k < p; k<<=1) 
	{
		int rpeer = rank - k;
		int speer = rank + k;

		// send or recv first ?
		if ((rank / k) & 1)
		{
			send_b(speer, p, b);
			buf = recv_b(rpeer);
		}
		else
		{
			buf = recv_b(rpeer);
			send_b(speer, p, b);
		}

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
