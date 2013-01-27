#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include "util/util.h"

#ifndef ATYPE
#define ATYPE int
#define ATYPE_MPI MPI_INT
#endif

#ifndef SEP
#define SEP ";"
#endif

#define LEFT 1
#define RIGHT 2
#define UP 3
#define DOWN 4

static int index_to_rank(int r, int c, int i, int j);
static void rank_to_index(int r, int c, int rank, int* i, int* j);
static void genmatrix(ATYPE* submat, int x, int y, int w, int h, int n);
static void stencil_middle(ATYPE* source, ATYPE* dest, int w, int h);
static void stencil_ring(ATYPE* source, ATYPE* dest, int w, int h);
static ATYPE* reference(int m, int n, double* dtime);

static void print_perf(int m, int n, int r, int c, int p, double dtime);
static void print_perf_debug(int m, int n, int r, int c, int p, double dtime);

static void* xmalloc(int size);
static void print_matrix(const char* caption, ATYPE* mat, int m, int n);
static int matrices_equal(ATYPE* m1, ATYPE* m2, int m, int n);
static void fail(int finalize);
static void fail_msg(int finalize, const char* format, ...);

int main(int argc, char* argv[])
{
	/*
	  Terminology:
		m: Number of rows in matrix,          e.g. 15
		n: Number of columns in matrix,       e.g. 8
		r: Number of block rows,              e.g. 5
		c: Number of block columns,           e.g. 4
		h: Height (rows) of a single block,   e.g. 3
		w: Width (columns) of a single block, e.g. 2
		i: Block Row index                    (0-4)
		j: Block Column index                 (0-3)
		x: Row index in matrix                (0-14)
		y: Column index in matrix             (0-7)
	 */
	int debug_flag;
	int m, n;
	int w, h;
	int i, j;
	int x, y;
	int rank, p;
	double time_start, time_end; // unused in nodes > 0
	ATYPE* result = NULL;

	if (parse_args(argc, argv, &debug_flag, "rows", &m, "columns", &n, NULL, NULL, NULL, NULL) != 0)
	{
		fail_msg(0, "Error parsing command args.\n");
	}

	if ((m < 1) || (n < 1))
	{
		fail_msg(0, "Bad input?\n");
	}

	result = xmalloc(m * n * sizeof(ATYPE));

    double dtime;
	ATYPE* refmat = reference(m, n, &dtime);
    print_perf(m, n, m, n, 1, dtime);
	free(refmat);




	return 0;
}


static int index_to_rank(int r, int c, int i, int j)
{
	if ((i < 0) || (j < 0) || (i >= r) || (j >= c))
	{
		return -1;
	}

	return i * c + j;
}

static void rank_to_index(int r, int c, int rank, int* i, int* j)
{
	if ((rank < 0) || (rank >= r * c))
	{
		*i = *j = -1;
	}

	*i = rank / c;
	*j = rank % c;
}

static void genmatrix(ATYPE* submat, int x, int y, int w, int h, int n)
{
	// NOTE: the submatrix has a margin of 1 element on every side
	//       to hold the elements to be communicated from neighbour
	//       nodes. genmatrix() temporarily zeroes them out.
	for (int yy = 0; yy < h+2; yy++)
	for (int xx = 0; xx < w+2; xx++)
	{
		long ii = yy * (w+2) + xx;
		long seq = (y+yy-1) * n + (x+xx-1);

		if ((0 == xx) || (0 == yy) || ((h+1) == yy) || ((w+1) == xx))
		{
			submat[ii] = 0;
		}
		else
		{
			// submat[ii] = (ATYPE) ((13L * seq * seq + seq * 17L + 4L) % 51L);
			submat[ii] = seq + 1;
			// submat[ii] = ((seq % 3) == 0);
			// if (xx == 1) submat[ii] = 1;
		}
	}
}

static void stencil_middle(ATYPE* source, ATYPE* dest, int w, int h)
{
	// NOTE: source is 2 rows and 2 columns larger than dest. re-center!
	for (int y = 1; y < h-1; y++)
	for (int x = 1; x < w-1; x++)
	{
		int d = y * w + x;
		int s = (y+1) * (w+2) + (x+1);
		dest[d] = (source[s-1] + source[s+1] + source[s-(w+2)] + source[s+(w+2)]) / 4;
	}
}

static void stencil_ring(ATYPE* source, ATYPE* dest, int w, int h)
{
	// NOTE: source is 2 rows and 2 columns larger than dest. re-center!
	for (int y = 0; y < h; y++)
	{
		int s = (y+1)*(w+2)+1;
		int x = w-1;
		dest[y*w] = (source[s-1] + source[s+1] + source[s-(w+2)] + source[s+(w+2)]) / 4;
		dest[y*w+x] = (source[s+x-1] + source[s+x+1] + source[s+x-(w+2)] + source[s+x+(w+2)]) / 4;
	}

	for (int x = 0; x < w; x++)
	{
		int s1 = (w+2)+x+1;
		int s2 = h*(w+2)+x+1;
		dest[x] = (source[s1-1] + source[s1+1] + source[s1-(w+2)] + source[s1+(w+2)]) / 4;
		dest[(h-1)*w+x] = (source[s2-1] + source[s2+1] + source[s2-(w+2)] + source[s2+(w+2)]) / 4;
	}
}

static ATYPE* reference(int m, int n, double* dtime)
{
	ATYPE* source = xmalloc((m+2) * (n+2) * sizeof(ATYPE));
	ATYPE* dest = xmalloc(m * n * sizeof(ATYPE));
	genmatrix(source, 0, 0, n, m, n);
    
    struct timespec start;
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &start);
	stencil_middle(source, dest, n, m);
	stencil_ring(source, dest, n, m);
    clock_gettime(CLOCK_REALTIME, &end);
	free(source);

    *dtime = (end.tv_nsec - start.tv_nsec)/1000000000.0;
	return dest;
}

static void print_perf(int m, int n, int r, int c, int p, double dtime)
{
	printf("%d%s%d%s%d%s%d%s%d%s%f\n", m, SEP, n, SEP, r, SEP, c, SEP, p, SEP, dtime);
}

static void print_perf_debug(int m, int n, int r, int c, int p, double dtime)
{
	printf("Input rows: %d\n", m);
	printf("Input columns: %d\n", n);
	printf("Block rows: %d\n", r);
	printf("Block columns: %d\n", c);
	printf("Nodes: %d\n", p);
	printf("Time: %f\n", dtime);
}


static void* xmalloc(int size)
{
	void* ret = malloc(size);
	if (NULL == ret)
	{
		fail_msg(0, "MALLOC ERROR: %s\n", strerror(errno));
	}
	else return ret;
}

static void print_matrix(const char* caption, ATYPE* mat, int m, int n)
{
	if (NULL != caption) printf("%s:\n", caption);

	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
		{
			printf("%3d  ", mat[i*n+j]);
		}
		printf("\n");
	}
}

static int matrices_equal(ATYPE* m1, ATYPE* m2, int m, int n)
{
	for (int i = 0; i < m*n; i++)
	{
		if (m1[i] != m2[i]) return 0;
	}

	return 1;
}

static void fail(int finalize)
{
	if (finalize)
	{
	}

	exit(1);
}

static void fail_msg(int finalize, const char* format, ...)
{
	va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

	if (finalize)
	{
	}

	exit(1);
}
