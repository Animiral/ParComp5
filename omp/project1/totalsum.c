#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include "util/util.h"

#ifndef ATYPE
#define ATYPE int
#endif

#ifndef SEP
#define SEP ";"
#endif

static ATYPE* geninput(int n);
static void benchmark(ATYPE data[], int n, ATYPE* sum, double* dtime);
static ATYPE totalsum(ATYPE a[], int n);
static ATYPE reference(int n);

static void* xmalloc(int size);
static void print_array(const char* caption, ATYPE a[], int n);
static int array_equal(ATYPE* a1, ATYPE* a2, int n);
static void print_perf(int n, int t, double dtime);
static void print_perf_debug(int n, int t, double dtime);
static void fail(const char* format, ...);

int main(int argc, char* argv[])
{
	int debug_flag;
	int n;
	int t;

	if (parse_args(argc, argv, &debug_flag, "input size", &n, "threads", &t, NULL, NULL, NULL, NULL) != 0)
	{
		fail("Error while parsing args.");
	}

	if ((n < 1) || (t < 1))
	{
		fail("Bad input");
	}

	omp_set_num_threads(t);

	ATYPE* data = geninput(n);

	if (debug_flag) print_array("input", data, n);

	ATYPE sum;
	double dtime;
	benchmark(data, n, &sum, &dtime);

	if (debug_flag)
	{
		/* ========== OUTPUT =========== */

		printf("output sum: %d\n", sum);
		print_perf_debug(n, t, dtime);

		ATYPE ref = reference(n);

		if (ref == sum)
		{
			printf("SUCCESS\n");
		}
		else
		{
			printf("EPIC FAILURE\n");
		}
	}
	else
	{
		print_perf(n, t, dtime);
	}
	
	free(data);
}


static ATYPE* geninput(int n)
{
	int i;
	ATYPE* a = xmalloc(n * sizeof(ATYPE));

	for (i = 0; i < n; i++)
	{
		// block[i] = (global_i % 3) == 0;
		a[i] = i + 1;
	}

	return a;
}

static void benchmark(ATYPE data[], int n, ATYPE* sum, double* dtime)
{
	double stime = omp_get_wtime();
	*sum = totalsum(data, n);
	double etime = omp_get_wtime();

	*dtime = etime - stime;
}

static ATYPE totalsum(ATYPE a[], int n)
{
	ATYPE sum = 0;

	#pragma omp parallel for reduction(+: sum)
	for (int i = 0; i < n; i++)
	{
		sum += a[i];
	}

	return sum;
}

static ATYPE reference(int n)
{
	int i;
	ATYPE* a = geninput(n);
	ATYPE sum = 0;

	for (i = 0; i < n; i++)
	{
		sum += a[i];
	}

	return sum;
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

static void print_array(const char* caption, ATYPE a[], int n)
{
	printf("%s:\n", caption);
	for (int i = 0; i < n; i++) 
	{
		printf("%d,", a[i]);
	}
	printf("\n");
}

static int array_equal(ATYPE* a1, ATYPE* a2, int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		if (a1[i] != a2[i]) return 0;
	}

	return 1;
}

static void print_perf(int n, int t, double dtime)
{
    // Print additional 0 ops counter to match other implementations.
	printf("%d%s%d%s%d%s%f\n", n, SEP, t, SEP, 0, SEP, dtime);
}

static void print_perf_debug(int n, int t, double dtime)
{
	printf("Input size: %d\n", n);
	printf("Threads: %d\n", t);
	printf("Time: %f\n", dtime);
}

static void fail(const char* format, ...)
{
	va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
	exit(1);
}
