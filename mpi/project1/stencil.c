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

static int index_to_rank(int r, int c, int i, int j);
static void rank_to_index(int r, int c, int rank, int* i, int* j);
static void genmatrix(ATYPE* submat, int x, int y, int w, int h, int n);
static void stencil(ATYPE* source, ATYPE* dest, int w, int h);
static ATYPE* reference(int m, int n);

static void send_row(ATYPE* mat, int w, int y, int peer);
static void send_col(ATYPE* mat, int w, int h, int x, int peer);
static void recv_buf(ATYPE* buf, int len, int peer);
static void buf_to_row(ATYPE* mat, int w, int y, ATYPE* buf);
static void buf_to_col(ATYPE* mat, int w, int h, int x, ATYPE* buf);

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

	/* ================ BEGIN OF ALGORITHM ================ */

	ATYPE* buffer;
	int ret;

	// columns stage 1: exchange between nodes (2*k) <--> (2*k+1)
	buffer = xmalloc(h * sizeof(ATYPE));

	if (j & 1) // communicate with left
	{
		// there is always a left neighbour -> no check
		int peer = rank-1;
		recv_buf(buffer, h, peer);
		send_col(source, w, h, 0, peer);
		buf_to_col(source, w, h, -1, buffer);
	}
	else // communicate with right
	{
		if (j < c-1)
		{
			int peer = rank+1;
			send_col(source, w, h, w-1, peer);
			recv_buf(buffer, h, peer);
		}
		else
		{
			memset(buffer, 0, h * sizeof(ATYPE));
		}

		buf_to_col(source, w, h, w, buffer);
	}

	// columns stage 2: exchange between nodes (2*k-1) <--> (2*k)
	if (j & 1) // communicate with right
	{
		if (j < c-1)
		{
			int peer = rank+1;
			recv_buf(buffer, h, peer);
			send_col(source, w, h, w-1, peer);
		}
		else
		{
			memset(buffer, 0, h * sizeof(ATYPE));
		}

		buf_to_col(source, w, h, w, buffer);
	}
	else // communicate with left
	{
		if (j > 0)
		{
			int peer = rank-1;
			send_col(source, w, h, 0, peer);
			recv_buf(buffer, h, peer);
		}
		else
		{
			memset(buffer, 0, h * sizeof(ATYPE));
		}

		buf_to_col(source, w, h, -1, buffer);
	}

	free(buffer);
	buffer = xmalloc(w * sizeof(ATYPE));

	// rows stage 1: exchange between nodes (2*k) <--> (2*k+1) [row index]
	if (i & 1) // communicate with upper
	{
		// there is always an upper neighbour -> no check
		int peer = rank-c;
		recv_buf(buffer, w, peer);
		send_row(source, w, 0, peer);
		buf_to_row(source, w, -1, buffer);
	}
	else // communicate with lower
	{
		if (i < r-1)
		{
			int peer = rank+c;
			send_row(source, w, h-1, peer);
			recv_buf(buffer, w, peer);
		}
		else
		{
			memset(buffer, 0, w * sizeof(ATYPE));
		}

		buf_to_row(source, w, h, buffer);
	}

	// rows stage 2: exchange between nodes (2*k-1) <--> (2*k) [row index]
	if (i & 1) // communicate with lower
	{
		if (i < r-1)
		{
			int peer = rank+c;
			recv_buf(buffer, w, peer);
			send_row(source, w, h-1, peer);
		}
		else
		{
			memset(buffer, 0, w * sizeof(ATYPE));
		}

		buf_to_row(source, w, h, buffer);
	}
	else // communicate with upper
	{
		if (i > 0)
		{
			int peer = rank-c;
			send_row(source, w, 0, peer);
			recv_buf(buffer, w, peer);
		}
		else
		{
			memset(buffer, 0, w * sizeof(ATYPE));
		}

		buf_to_row(source, w, -1, buffer);
	}

	free(buffer);

	// ACTION!

	stencil(source, dest, w, h);

	/* ========== GATHER =========== */

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
	}
	else
	{
		MPI_Send(dest, w*h, ATYPE_MPI, 0, rank, MPI_COMM_WORLD);
	}

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

	/* ========== OUTPUT =========== */

	if (debug_flag && (0 == rank))
	{
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
			submat[ii] = seq + 1; // (ATYPE) ((13L * seq * seq + seq * 17L + 4L) % 51L);
		}
	}
}

static void stencil(ATYPE* source, ATYPE* dest, int w, int h)
{
	// NOTE: source is 2 rows and 2 columns larger than dest. re-center!
	for (int y = 0; y < h; y++)
	for (int x = 0; x < w; x++)
	{
		int d = y * w + x;
		int s = (y+1) * (w+2) + (x+1);
		dest[d] = (source[s-1] + source[s+1] + source[s-(w+2)] + source[s+(w+2)]) / 4;
	}
}

static ATYPE* reference(int m, int n)
{
	ATYPE* source = xmalloc((m+2) * (n+2) * sizeof(ATYPE));
	ATYPE* dest = xmalloc(m * n * sizeof(ATYPE));
	genmatrix(source, 0, 0, n, m, n);
	stencil(source, dest, n, m);
	//print_matrix("Reference result", dest, m, n);
	free(source);
	return dest;
}


static void send_row(ATYPE* mat, int w, int y, int peer)
{
	// NOTE: mind the 1-column, 1-row margin on mat
	ATYPE* buf = &mat[(y+1)*(w+2)+1];
	int ret = MPI_Send(buf, w, ATYPE_MPI, peer, 0, MPI_COMM_WORLD);
}

static void send_col(ATYPE* mat, int w, int h, int x, int peer)
{
	// NOTE: mind the 1-column, 1-row margin on mat
	ATYPE* buf = xmalloc(h * sizeof(ATYPE));
	
	for (int y = 0; y < h; y++)
	{
		buf[y] = mat[(y+1)*(w+2)+x+1];
	}

	int ret = MPI_Send(buf, h, ATYPE_MPI, peer, 0, MPI_COMM_WORLD);

	free(buf);
}

static void recv_buf(ATYPE* buf, int len, int peer)
{
	int ret = MPI_Recv(buf, len, ATYPE_MPI, peer, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

static void buf_to_row(ATYPE* mat, int w, int y, ATYPE* buf)
{
	// NOTE: y here is still the row index of the block, but as
	//       it will likely point to the margin rows of the 'source'
	//       matrix, the values of -1 and h are legitimate.
	ATYPE* begin = &mat[(y+1)*(w+2)+1];
	memcpy(begin, buf, w * sizeof(ATYPE));
}

static void buf_to_col(ATYPE* mat, int w, int h, int x, ATYPE* buf)
{
	// NOTE: x here is still the column index of the block, but as
	//       it will likely point to the margin rows of the 'source'
	//       matrix, the values of -1 and w are legitimate.
	for (int y = 0; y < h; y++)
	{
		mat[(y+1)*(w+2)+x+1] = buf[y];
	}
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
