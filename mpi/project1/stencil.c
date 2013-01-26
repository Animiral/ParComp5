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

#define LEFT 1
#define RIGHT 2
#define UP 3
#define DOWN 4

static int index_to_rank(int r, int c, int i, int j);
static void rank_to_index(int r, int c, int rank, int* i, int* j);
static void genmatrix(ATYPE* submat, int x, int y, int w, int h, int n);
static void stencil_middle(ATYPE* source, ATYPE* dest, int w, int h);
static void stencil_ring(ATYPE* source, ATYPE* dest, int w, int h);
static ATYPE* reference(int m, int n);

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
	int c, r;
	int w, h;
	int i, j;
	int x, y;
	int rank, p;
	double time_start, time_end; // unused in nodes > 0
	ATYPE* result = NULL;

	if (parse_args(argc, argv, &debug_flag, "rows", &m, "columns", &n, "number of block columns", &c, NULL, NULL) != 0)
	{
		fail_msg(0, "Error parsing command args.\n");
	}

	if ((m < 1) || (n < 1) || (c < 1))
	{
		fail_msg(0, "Bad input?\n");
	}

	if (c > n)
	{
		debug_flag ? fail_msg(1, "ERROR: matrix with %d columns cannot be divided into %d columns.\n", n, c) : fail(1);
	}

	MPI_Init (&argc, &argv);

	// get rank and size from communicator
	MPI_Comm_size (MPI_COMM_WORLD, &p);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);

	if (0 == rank)
	{
		time_start = MPI_Wtime();
	}

	r = p / c;

	if (r < 1) 
	{
		debug_flag ? fail_msg(1, "Bad input?\n") : fail(1);
	}

	if (r * c != p)
	{
		debug_flag ? fail_msg(1, "ERROR: %d rows * %d columns cannot be exactly fit to %d nodes.\n", r, c, p) : fail(1);
	}

	if (r > m)
	{
		debug_flag ? fail_msg(1, "ERROR: matrix with %d rows cannot be divided into %d rows.\n", m, r) : fail(1);
	}

	// width and height of a block
	w = n / c;
	h = m / r;

	rank_to_index(r, c, rank, &i, &j);

	if ((i < 0) || (j < 0))
	{
		fail_msg(1, "ERROR: this node (%d) does not have an assigned matrix block.\n", rank);
	}

	// start indices of this node's submatrix
	x = j * w;
	y = i * h;

	// allocate my submatrix (twice)
	ATYPE* source = xmalloc((w+2) * (h+2) * sizeof(ATYPE));
	ATYPE* dest = xmalloc(w * h * sizeof(ATYPE));
	genmatrix(source, x, y, w, h, n);

	/* ================ COMMUNICATION ================ */

	int ret;

	MPI_Request request[8];
	MPI_Datatype column_type;
	MPI_Type_vector(h, 1, w+2, ATYPE_MPI, &column_type);
	MPI_Type_commit(&column_type);

	int l_peer = rank-1; if (j <= 0)   l_peer = MPI_PROC_NULL;
	int r_peer = rank+1; if (j >= c-1) r_peer = MPI_PROC_NULL;
	int u_peer = rank-c; if (i <= 0)   u_peer = MPI_PROC_NULL;
	int d_peer = rank+c; if (i >= r-1) d_peer = MPI_PROC_NULL;

	ret = MPI_Isend(&source[(w+2)+1],       1, column_type, l_peer, LEFT,  MPI_COMM_WORLD, &request[0]);
	ret = MPI_Isend(&source[(w+2)+w],       1, column_type, r_peer, RIGHT, MPI_COMM_WORLD, &request[1]);
	ret = MPI_Isend(&source[(w+2)+1],       w, ATYPE_MPI,   u_peer, UP,    MPI_COMM_WORLD, &request[2]);
	ret = MPI_Isend(&source[h*(w+2)+1],     w, ATYPE_MPI,   d_peer, DOWN,  MPI_COMM_WORLD, &request[3]);

	ret = MPI_Irecv(&source[(w+2)],         1, column_type, l_peer, RIGHT, MPI_COMM_WORLD, &request[4]);
	ret = MPI_Irecv(&source[(w+2)+w+1],     1, column_type, r_peer, LEFT,  MPI_COMM_WORLD, &request[5]);
	ret = MPI_Irecv(&source[1],             w, ATYPE_MPI,   u_peer, DOWN,  MPI_COMM_WORLD, &request[6]);
	ret = MPI_Irecv(&source[(h+1)*(w+2)+1], w, ATYPE_MPI,   d_peer, UP,    MPI_COMM_WORLD, &request[7]);

	/* ================ CALCULATION ================ */

	stencil_middle(source, dest, w, h);
	ret = MPI_Waitall(8, request, MPI_STATUS_IGNORE);
	stencil_ring(source, dest, w, h);

	/* ========== PERFORMANCE MEASUREMENTS =========== */

	if (0 == rank)
	{
		time_end = MPI_Wtime();

		if (debug_flag)
		{
			print_perf_debug(m, n, r, c, p, time_end - time_start);
		}
		else
		{
			print_perf(m, n, r, c, p, time_end - time_start);
		}
	}

	/* ================ GATHER ================ */

	if (debug_flag)
	{
		if (0 == rank)
		{
			// TODO: use MPI gather operations
			result = xmalloc(m * n * sizeof(ATYPE));

			for (int yy = 0; yy < h; yy++)
			for (int xx = 0; xx < w; xx++)
			{
				result[yy*n+xx] = dest[yy*w+xx];
			}

			for (int peer = 1; peer < p; peer++)
			{
				MPI_Recv(dest, w*h, ATYPE_MPI, peer, peer, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				int pi, pj;
				rank_to_index(r, c, peer, &pi, &pj);

				for (int yy = 0; yy < h; yy++)
				for (int xx = 0; xx < w; xx++)
				{
					result[(yy+pi*h)*n+(xx+pj*w)] = dest[yy*w+xx];
				}
			}

			/* ========== OUTPUT =========== */

			//print_matrix("Result", matrix, m, n);
			ATYPE* refmat = reference(m, n);
			if (matrices_equal(result, refmat, m, n) != 0)
			{
				printf("SUCCESS\n");
			}
			else
			{
				printf("EPIC FAILURE\n");
			}
			free(refmat);
		}
		else
		{
			MPI_Send(dest, w*h, ATYPE_MPI, 0, rank, MPI_COMM_WORLD);
		}
	}

	MPI_Type_free(&column_type);
	free(result);
	free(source);
	free(dest);

	MPI_Finalize();

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

static ATYPE* reference(int m, int n)
{
	ATYPE* source = xmalloc((m+2) * (n+2) * sizeof(ATYPE));
	ATYPE* dest = xmalloc(m * n * sizeof(ATYPE));
	genmatrix(source, 0, 0, n, m, n);
	stencil_middle(source, dest, n, m);
	stencil_ring(source, dest, n, m);
	free(source);
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
		MPI_Finalize();
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
		MPI_Finalize();
	}

	exit(1);
}
