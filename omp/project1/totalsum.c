#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include "util/args.h"

#ifndef ATYPE
#define ATYPE int
#endif

static void benchmark(ATYPE data[], int n);
static int totalsum(ATYPE a[], int n);
static void print_array();
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

	for (int i = 0; i < n; i++) 
	{
		data[i] = (i % 3 == 0);
	}

	if (debug_flag) print_array("input", data, n);
	benchmark(data, n);

	if (debug_flag) print_array("output", data, n);
	print_perf(n, t, 0);
}

void benchmark(ATYPE data[], int n)
{
	double stime = omp_get_wtime();
	int tsum = totalsum(data, n);
	double etime = omp_get_wtime();

	printf("Total sum: %d\n", tsum);
	printf("Time: %f\n", etime - stime);
}

int totalsum(ATYPE a[], int n)
{
	int sum = 0;

	#pragma omp parallel for reduction(+: sum)
	for (int i = 0; i < n; i++)
	{
		sum += a[i];
	}

	return sum;
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
