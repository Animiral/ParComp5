#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <unistd.h>
#include <time.h>
#include "util/util.h"

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

	if (parse_args(argc, argv, &debug_flag, "input size", &n, NULL, NULL, NULL, NULL, NULL, NULL) != 0)
	{
		exit(1);
	}

	if (n < 1)
	{
		fprintf(stderr, "%s\n", "bad input?");
		exit(1);
	}


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
	if (debug_flag) print_perf_debug(n, 1);
	else print_perf(n, 1, dtime);
}

void benchmark(ATYPE data[], int n, ATYPE* sum, double* dtime)
{
    struct timespec start;
    struct timespec end;
    clock_gettime(CLOCK_REALTIME, &start);
	*sum = totalsum(data, n);
    clock_gettime(CLOCK_REALTIME, &end);

	*dtime = (end.tv_nsec - start.tv_nsec)/1000000000.0;
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
    // Print additional 0 ops counter to match other implementations.
	printf("%d%s%d%s%d%s%f\n", n, SEP, t, SEP, 0, SEP, dtime);
}

static void print_perf_debug(int n, int t)
{
	printf("Input size: %d\n", n);
	printf("Threads: %d\n", t);
}
