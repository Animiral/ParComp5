#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <errno.h>
#include "util/args.h"

#ifndef ATYPE
#define ATYPE int
#define ATYPE_MPI MPI_INT
#endif

#ifndef SEP
#define SEP ";"
#endif

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

static int index_to_rank(int r, int c, int i, int j);
static void rank_to_index(int r, int c, int rank, int* i, int* j);
static void genmatrix(ATYPE* submat, int x, int y, int w, int h);
static void stencil(ATYPE* source, ATYPE* dest, int w, int h);

static void send_row(ATYPE* mat, int w, int y, int peer);
static void send_col(ATYPE* mat, int h, int x, int peer);
static void recv_buf(ATYPE* buf, int len, int peer);

static void* xmalloc(int size);

int main(int argc, char* argv[])
{
	int debug_flag;
	int m;
	int n;
	int c;

	if (parse_args(argc, argv, &debug_flag, "rows", &m, "columns", &n, "number of block columns", &c, NULL, NULL) != 0)
	{
		exit(1);
	}

	if ((m < 1) || (n < 1) || (c < 1))
	{
		fprintf(stderr, "%s\n", "bad input?");
		exit(1);
	}

	int rank, p;
	char name[MPI_MAX_PROCESSOR_NAME];
	int nlen;

	MPI_Init (&argc, &argv);

	// get rank and size from communicator
	MPI_Comm_size (MPI_COMM_WORLD, &p);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);

	MPI_Get_processor_name (name, &nlen);

	int r = p / c;

	if (r < 1) 
	{
		fprintf(stderr, "%s\n", "bad input?");
		exit(1);
	}

	if (r * c != p)
	{
		fprintf(stderr, "ERROR: %d rows * %d columns cannot be exactly fit to %d nodes.\n", r, c, p);
		exit(1);
	}

	if (r > m)
	{
		fprintf(stderr, "ERROR: matrix with %d rows cannot be divided into %d rows.\n", m, r);
		exit(1);
	}

	if (c > n)
	{
		fprintf(stderr, "ERROR: matrix with %d columns cannot be divided into %d columns.\n", n, c);
		exit(1);
	}

	// width and height of a block
	int w = n / c;
	int h = m / r;

	int i, j; // this node's block index in the matrix
	rank_to_index(r, c, rank, &i, &j);

	if ((i < 0) || (j < 0))
	{
		fprintf(stderr, "ERROR: this node (%d) does not have an assigned matrix block.\n", rank);
		exit(1);
	}

	// start indices of this node's submatrix
	int x = j * w;
	int y = i * h;

	// allocate my submatrix (twice)
	ATYPE* source = xmalloc(w * h * sizeof(ATYPE));
	genmatrix(source, x, y, w, h);
	ATYPE* dest = xmalloc(w * h * sizeof(ATYPE));

	/* ================ BEGIN OF ALGORITHM ================ */

	stencil(source, dest, w, h);

	ATYPE* buffer;
	int ret;

	// columns stage 1: exchange between nodes (2*k) <--> (2*k+1)
	buffer = xmalloc(h * sizeof(ATYPE));

	if (i & 1)
	{
		int peer = rank-1;

		recv_buf(buffer, h, peer);
		send_col(source, h, 0, peer);

		// TODO: computation
	}
	else
	{
		int peer = rank+1;

		if (i != h-1)
		{
			send_col(source, h, w-1, peer);
			recv_buf(buffer, h, peer);
		}
		else
		{
			memset(buffer, 0, h * sizeof(ATYPE));
		}

		// TODO: computation
	}

	free(buffer);
	buffer = NULL;

	// columns stage 2: exchange between nodes (2*k-1) <--> (2*k)

	// rows stage 1: exchange between nodes (2*k) <--> (2*k+1) [row index]

	// rows stage 2: exchange between nodes (2*k-1) <--> (2*k) [row index]

/*
	if (rank==0) {
		printf("Rank %d initializing, total %d\n",rank,size);
	} else {
		MPI_Recv(&prev,1,MPI_INT,rank-1,HELLO,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		printf("Rank %d on %s received from %d, passing on\n",rank,name,prev);
	}
	if (rank+1<size) 
		{
			MPI_Send(&rank,1,MPI_INT,rank+1,HELLO,MPI_COMM_WORLD);
*/

	/* ========== OUTPUT =========== */

	if (debug_flag)
	{
		if (rank == 0)
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

			for (int ii = 0; ii < m; ii++) 
			{
				for (int jj = 0; jj < n; jj++)
				{
					printf("%d  ", matrix[ii*n+jj]);
				}
				printf("\n");
			}

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

static void genmatrix(ATYPE* submat, int x, int y, int w, int h)
{
	for (int yy = 0; yy < h; yy++)
	for (int xx = 0; xx < w; xx++)
	{
		long ii = (yy + y) * w + (xx + x);
		submat[ii] = (ATYPE) ((13L * ii * ii + ii * 17L + 4L) % 51L);
	}
}

static void stencil(ATYPE* source, ATYPE* dest, int w, int h)
{
	for (int y = 1; y < h-1; y++)
	for (int x = 1; x < w-1; x++)
	{
		int i = y * w + x;
		dest[i] = (source[i-1] + source[i+1] + source[i-w] + source[i+w]) / 4;
	}
}

static void send_row(ATYPE* mat, int w, int y, int peer)
{
	ATYPE* buf = &mat[y*w];
	int ret = MPI_Send(buf, w, ATYPE_MPI, peer, 0, MPI_COMM_WORLD);
}

static void send_col(ATYPE* mat, int h, int x, int peer)
{
	ATYPE* buf = xmalloc(h * sizeof(ATYPE));
	
	for (int y = 0; y < h; y++)
	{
		buf[y] = mat[y*h+x];
	}

	int ret = MPI_Send(buf, h, ATYPE_MPI, peer, 0, MPI_COMM_WORLD);

	free(buf);
}

static void recv_buf(ATYPE* buf, int len, int peer)
{
	int ret = MPI_Recv(buf, len, ATYPE_MPI, peer, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

static void* xmalloc(int size)
{
	void* ret = malloc(size);
	if (NULL == ret)
	{
		fprintf(stderr, "MALLOC ERROR: %s\n", strerror(errno));
		exit(1);
	}
	else return ret;
}
