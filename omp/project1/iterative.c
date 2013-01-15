#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>

#ifndef ATYPE
#define ATYPE int
#endif

static void benchmark(ATYPE data[], int n, int* ops);
static void scan();
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
	int ops = 0;

	for (int i = 0; i < n; i++) 
	{
		data[i] = (i % 3 == 0);
	}

	//print_array("input", data, n);
	benchmark(data, n, &ops);

	//print_array("output", data, n);
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
