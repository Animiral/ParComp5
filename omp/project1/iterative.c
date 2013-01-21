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

static void benchmark(ATYPE data[], int n, int* ops, double* dtime);
static void scan();
static void print_array();
static void print_perf(int n, int t, int ops, double dtime);
static void print_perf_debug(int n, int t, int ops);

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

	double dtime;
	benchmark(data, n, &ops, &dtime);	

	if (debug_flag) printf("Time: %f\n", dtime);
	if (debug_flag) print_array("output", data, n);
	if (debug_flag) print_perf_debug(n, t, ops);
	else print_perf(n, t, ops, dtime);
}

void benchmark(ATYPE data[], int n, int* ops, double* dtime)
{
	double stime = omp_get_wtime();
	scan(data, n, ops);
	double etime = omp_get_wtime();

	*dtime = etime - stime;
}

void scan(ATYPE a[], int n, int* ops)
{
	int r;
	int r2;
	int s = 0;
	int pc_ops;

	// phase 0
	for (r = 1; r < n; r = r2)
	{
		r2 = r*2;
		pc_ops = 0;

		#pragma omp parallel for reduction(+: pc_ops)
		for (s = r2-1; s < n; s += r2)
		{
			a[s] = a[s-r] + a[s];
			pc_ops++;
		}

		*ops += pc_ops;
	}

	// phase 1
	for (r = r/2; r > 1; r = r2)
	{
		r2 = r/2;
		pc_ops = 0;

		#pragma omp parallel for reduction(+: pc_ops)
		for (s = r-1; s < n-r2; s += r)
		{
			a[s+r2] = a[s] + a[s+r2];
			pc_ops++;
		}

		*ops += pc_ops;
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

static void print_perf(int n, int t, int ops, double dtime)
{
	printf("%d%s%d%s%d%s%f\n", n, SEP, t, SEP, ops, SEP, dtime);
}

static void print_perf_debug(int n, int t, int ops)
{
	printf("Input size: %d\n", n);
	printf("Threads: %d\n", t);
	printf("Total operations: %d\n", ops);
}
