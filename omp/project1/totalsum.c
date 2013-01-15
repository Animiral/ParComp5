#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>

#ifndef ATYPE
#define ATYPE int
#endif

static void benchmark(ATYPE data[], int n);
static int totalsum(ATYPE a[], int n);
static void print_usage();
static void print_array();
static void print_perf(int n, int t, int ops);

int main(int argc, char* argv[])
{
	if (argc != 3) 
	{
		print_usage();
		exit(1);
	}

	int n = strtol(argv[1], NULL, 0);
	int t = strtol(argv[2], NULL, 0);

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

	//print_array("input", data, n);
	benchmark(data, n);

	//print_array("output", data, n);
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

void print_usage() 
{
	fprintf(stderr, "%s\n", "Arguments: <input size> <threads>");
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
