#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include "util/args.h"

#ifndef ATYPE
#define ATYPE int
#endif

static void benchmark(ATYPE** data, int n, int* ops);
static void scan(ATYPE** in, int n, int* ops);
static void print_array(const char* caption, ATYPE a[], int n);
static void print_perf(int n, int t, int ops);

int main(int argc, char* argv[])
{
	int debug_flag;
	int n;
	int t;

	if (parse_args(argc, argv, &debug_flag, "input size", &n, "threads", &t, NULL, NULL, NULL, NULL) != 0)
	{
		exit(1);
	}

	if ((n < 1) || (t < 1))
	{
		fprintf(stderr, "%s\n", "bad input?");
		exit(1);
	}

	omp_set_num_threads(t);

	ATYPE* data = malloc(sizeof(ATYPE) * n);
	int ops = 0;

	for (int i = 0; i < n; i++) 
	{
		data[i] = (i % 3 == 0);
	}

	if (debug_flag) print_array("input", data, n);
	benchmark(&data, n, &ops);	

	if (debug_flag) print_array("output", data, n);
	print_perf(n, t, ops);
}

void benchmark(ATYPE** data, int n, int* ops)
{
	double stime = omp_get_wtime();
	scan(data, n, ops);
	double etime = omp_get_wtime();

	printf("Time: %f\n", etime - stime);
}

void scan(ATYPE** in, int n, int* ops)
{
	int i;
	int k;
	ATYPE* a;
	ATYPE* b;
	ATYPE* temp;
	int pc_ops;

	a = *in;
	b = malloc(sizeof(ATYPE) * n);

	for (k = 1; k < n; k<<=1) 
	{
		pc_ops = 0;

		#pragma omp parallel for
		for (i = 0; i < k; i++)
		{
			b[i] = a[i];
		}

		#pragma omp parallel for reduction(+: pc_ops)
		for (i = k; i < n; i++)
		{
			b[i] = a[i] + a[i-k];
			pc_ops++;
		}

		*ops += pc_ops;

		// swappity
		temp = a;
		a = b;
		b = temp;
	}

	*in = a;
}

void print_array(const char* caption, ATYPE a[], int n)
{
	printf("%s:\n", caption);
	for (int i = 0; i < n; i++) 
	{
		printf("%d,", a[i]);
	}
	printf("\n");
}

static void print_perf(int n, int t, int ops)
{
	printf("Input size: %d\n", n);
	printf("Threads: %d\n", t);
	printf("Total operations: %d\n", ops);
}