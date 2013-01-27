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

// TODO: Additional performance counters for array access (in addition to addition counters)

static ATYPE* geninput(int n);
static void benchmark(ATYPE data[], int n, int* plus_ops, int* acc_ops, double* dtime);
static void scan(ATYPE a[], int n, int* plus_ops, int* acc_ops);
static ATYPE* reference(int n);

static void* xmalloc(int size);
static void print_array(const char* caption, ATYPE a[], int n);
static int array_equal(ATYPE* a1, ATYPE* a2, int n);
static void print_perf(int n, int t, int plus_ops, int acc_ops, double dtime);
static void print_perf_debug(int n, int t, int plus_ops, int acc_ops, double dtime);
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
	int plus_ops = 0;
	int acc_ops = 0;

	if (debug_flag) print_array("input", data, n);

	double dtime;
	benchmark(data, n, &plus_ops, &acc_ops, &dtime);

	if (debug_flag)
	{
		/* ========== OUTPUT =========== */

		print_array("output", data, n);
		print_perf_debug(n, t, plus_ops, acc_ops, dtime);

		ATYPE* ref = reference(n);

		if (array_equal(data, ref, n))
		{
			printf("SUCCESS\n");
		}
		else
		{
			printf("EPIC FAILURE\n");
		}

		free(ref);
	}
	else
	{
		print_perf(n, t, plus_ops, acc_ops, dtime);
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

static void benchmark(ATYPE data[], int n, int* plus_ops, int* acc_ops, double* dtime)
{
	double stime = omp_get_wtime();
	scan(data, n, plus_ops, acc_ops);
	double etime = omp_get_wtime();

	*dtime = etime - stime;
}

static void scan(ATYPE a[], int n, int* plus_ops, int* acc_ops)
{
	int i;
	ATYPE* b;
	int p_ops;
	int a_ops;

	if (n <= 1) return;

	b = alloca(sizeof(ATYPE) * (n/2));
	p_ops = 0;
	a_ops = 0;

	#pragma omp parallel for reduction(+: p_ops) reduction(+: a_ops)
	for (i = 0; i < n/2; i++) 
	{
		b[i] = a[2*i] + a[2*i+1];
		p_ops++;
		a_ops+=3;
	}

	*plus_ops += p_ops;
	*acc_ops += a_ops;

	scan(b, n/2, plus_ops, acc_ops);

	a[1] = b[0];
	p_ops = 0;
	a_ops = 0;

	#pragma omp parallel for reduction(+: p_ops) reduction(+: a_ops)
	for (i = 1; i < n/2; i++) {
		a[2*i] = b[i-1] + a[2*i];
		a[2*i+1] = b[i];
		p_ops++;
		a_ops+=5;
	}

	*plus_ops += p_ops;
	*acc_ops += a_ops;

	if (n % 2)
	{
		a[n-1] = b[n/2-1] + a[n-1];
		(*plus_ops)++;
		(*acc_ops)+=3;
	}
}

static ATYPE* reference(int n)
{
	int i;
	ATYPE* a = geninput(n);

	for (i = 1; i < n; i++)
	{
		a[i] += a[i-1];
	}

	return a;
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

static void print_perf(int n, int t, int plus_ops, int acc_ops, double dtime)
{
	printf("%d%s%d%s%d%s%d%s%f\n", n, SEP, t, SEP, plus_ops, SEP, acc_ops, SEP, dtime);
}

static void print_perf_debug(int n, int t, int plus_ops, int acc_ops, double dtime)
{
	printf("Input size: %d\n", n);
	printf("Threads: %d\n", t);
	printf("Plus operations: %d\n", plus_ops);
	printf("Access operations: %d\n", acc_ops);
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

