#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include "util/args.h"

#ifndef ATYPE
#define ATYPE int
#endif

// TODO: Additional performance counters for array access (in addition to addition counters)

static void benchmark(ATYPE data[], int n, int* ops);
static void scan(ATYPE a[], int n, int* ops);
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

	ATYPE data[n];
	int ops = 0;

	for (int i = 0; i < n; i++) 
	{
		data[i] = (i % 3 == 0);
	}

	if (debug_flag) print_array("input", data, n);
	benchmark(data, n, &ops);	

	if (debug_flag) print_array("output", data, n);
	print_perf(n, t, ops);
}

void benchmark(ATYPE data[], int n, int* ops)
{
	double stime = omp_get_wtime();
	scan(data, n, ops);
	double etime = omp_get_wtime();

	printf("Time: %f\n", etime - stime);
}

void scan(ATYPE a[], int n, int* ops)
{
	int i;
	ATYPE* b;
	int pc_ops;

	if (n <= 1) return;

	b = alloca(sizeof(ATYPE) * (n/2));
	pc_ops = 0;

	#pragma omp parallel for reduction(+: pc_ops)
	for (i = 0; i < n/2; i++) 
	{
		b[i] = a[2*i] + a[2*i+1];
		pc_ops++;
	}

	*ops += pc_ops;

	scan(b, n/2, ops);

	a[1] = b[0];
	pc_ops = 0;

	#pragma omp parallel for reduction(+: pc_ops)
	for (i = 1; i < n/2; i++) {
		a[2*i] = b[i-1] + a[2*i];
		a[2*i+1] = b[i];
		pc_ops++;
	}

	*ops += pc_ops;

	if (n % 2)
	{
		a[n-1] = b[n/2-1] + a[n-1];
		(*ops)++;
	}
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
