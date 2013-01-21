#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include "util/args.h"

#ifndef ATYPE
#define ATYPE int
#endif

#ifndef SEP
#define SEP ";"
#endif

static void benchmark(ATYPE data[], int n, ATYPE* sum, double* dtime);
static int totalsum(ATYPE a[], int n);
static void print_array();
static void print_perf(int n, int t, double dtime);
static void print_perf_debug(int n, int t);

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

	ATYPE sum;
	double dtime;
	benchmark(data, n, &sum, &dtime);

	if (debug_flag) printf("Total sum: %d\n", sum);
	if (debug_flag) printf("Time: %f\n", dtime);
	if (debug_flag) print_array("output", data, n);
	if (debug_flag) print_perf_debug(n, t);
	else print_perf(n, t, dtime);
}

void benchmark(ATYPE data[], int n, ATYPE* sum, double* dtime)
{
	double stime = omp_get_wtime();
	*sum = totalsum(data, n);
	double etime = omp_get_wtime();

	*dtime = etime - stime;
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

static void print_perf(int n, int t, double dtime)
{
	printf("%d%s%d%s%f\n", n, SEP, t, SEP, dtime);
}

static void print_perf_debug(int n, int t)
{
	printf("Input size: %d\n", n);
	printf("Threads: %d\n", t);
}
