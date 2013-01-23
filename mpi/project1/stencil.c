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

static int index_to_rank(int r, int c, int i, int j);
static void rank_to_index(int r, int c, int rank, int* i, int* j);
static void genmatrix(ATYPE* submat, int x, int y, int w, int h, int n);
static void stencil(ATYPE* source, ATYPE* dest, int w, int h);

static void send_row(ATYPE* mat, int w, int y, int peer);
static void send_col(ATYPE* mat, int w, int h, int x, int peer);
static void recv_buf(ATYPE* buf, int len, int peer);
static void buf_to_row(ATYPE* mat, int w, int y, ATYPE* buf);
static void buf_to_col(ATYPE* mat, int w, int h, int x, ATYPE* buf);

static void* xmalloc(int size);
static void print_matrix(const char* caption, ATYPE* mat, int m, int n);
static void fail(const char* format, ...);

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
	char name[MPI_MAX_PROCESSOR_NAME];
	int nlen;

	if (parse_args(argc, argv, &debug_flag, "rows", &m, "columns", &n, "number of block columns", &c, NULL, NULL) != 0)
	{
		fail("Error parsing command args.\n");
	}

	if ((m < 1) || (n < 1) || (c < 1))
	{
		fail("Bad input?.\n");
	}

	MPI_Init (&argc, &argv);

	// get rank and size from communicator
	MPI_Comm_size (MPI_COMM_WORLD, &p);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);

	MPI_Get_processor_name (name, &nlen);

	r = p / c;

	if (r < 1) 
	{
		fail("Bad input?.\n");
	}

	if (r * c != p)
	{
		fail("ERROR: %d rows * %d columns cannot be exactly fit to %d nodes.\n", r, c, p);
	}

	if (r > m)
	{
		fail("ERROR: matrix with %d rows cannot be divided into %d rows.\n", m, r);
	}

	if (c > n)
	{
		fail("ERROR: matrix with %d columns cannot be divided into %d columns.\n", n, c);
	}

	// width and height of a block
	w = n / c;
	h = m / r;

	rank_to_index(r, c, rank, &i, &j);

	if ((i < 0) || (j < 0))
	{
		fail("ERROR: this node (%d) does not have an assigned matrix block.\n", rank);
	}

	// start indices of this node's submatrix
	x = j * w;
	y = i * h;

	// allocate my submatrix (twice)
	ATYPE* source = xmalloc((w+2) * (h+2) * sizeof(ATYPE));
	genmatrix(source, x, y, w, h, n);
	ATYPE* dest = xmalloc(w * h * sizeof(ATYPE));

	/* ================ BEGIN OF ALGORITHM ================ */

	ATYPE* buffer;
	int ret;

	// columns stage 1: exchange between nodes (2*k) <--> (2*k+1)
	buffer = xmalloc(h * sizeof(ATYPE));

	if (i & 1)
	{
		int peer = rank-1;

		recv_buf(buffer, h, peer);
		send_col(source, w, h, 0, peer);
		buf_to_col(source, w, h, -1, buffer);
	}
	else
	{
		int peer = rank+1;

		if (i != c-1)
		{
			send_col(source, w, h, w-1, peer);
			recv_buf(buffer, h, peer);
			buf_to_col(source, w, h, w, buffer);
		}
		else
		{
			memset(buffer, 0, h * sizeof(ATYPE));
		}
	}

	free(buffer);
	buffer = NULL;

	// columns stage 2: exchange between nodes (2*k-1) <--> (2*k)

	// rows stage 1: exchange between nodes (2*k) <--> (2*k+1) [row index]

	// rows stage 2: exchange between nodes (2*k-1) <--> (2*k) [row index]


	// ACTION!

	if (debug_flag && (0 == rank))
	{
		print_matrix("Source", source, w+2, h+2);
	}

	stencil(source, dest, w, h);


	/* ========== OUTPUT =========== */

	if (debug_flag)
	{
		if (0 == rank)
		{
			ATYPE* matrix = xmalloc(m * n * sizeof(ATYPE));

			for (int yy = 0; yy < h; yy++)
			for (int xx = 0; xx < w; xx++)
			{
				matrix[yy*n+xx] = dest[yy*w+xx];
			}

			for (int peer = 1; peer < p; peer++)
			{
				MPI_Recv(dest, w*h, ATYPE_MPI, peer, peer, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

				int pi, pj;
				rank_to_index(r, c, peer, &pi, &pj);

				for (int yy = 0; yy < h; yy++)
				for (int xx = 0; xx < w; xx++)
				{
					matrix[(yy+pi*h)*n+(xx+pj*w)] = dest[yy*w+xx];
				}
			}
			
			print_matrix(NULL, matrix, m, n);

			printf("DONE\n");
		}
		else
		{
			MPI_Send(dest, w*h, ATYPE_MPI, 0, rank, MPI_COMM_WORLD);
		}
	}

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

		if ((0 == xx) || (0 == yy) || (h+1 == xx) || (w+1 == y))
		{
			submat[ii] = 0;
		}
		else
		{
			submat[ii] = (ATYPE) ((13L * seq * seq + seq * 17L + 4L) % 51L);
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

static void* xmalloc(int size)
{
	void* ret = malloc(size);
	if (NULL == ret)
	{
		fail("MALLOC ERROR: %s\n", strerror(errno));
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

static void fail(const char* format, ...)
{
	va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
	exit(1);
}
