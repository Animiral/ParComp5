#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include "util/util.h"

static void genpow2(int lower, int upper);
static void genprime(int lower, int upper);

static void* xmalloc(int size);
static void fail(const char* format, ...);

int main(int argc, char* argv[])
{
	int debug_flag;
	int lower; // inclusive lower boundary for generated numbers
	int upper; // inclusive upper boundary for generated numbers

	if (0 != parse_args(argc, argv, &debug_flag, "lower boundary", &lower, "upper boundary", &upper, NULL, NULL, NULL, NULL))
	{
		fail("Error parsing command args.\n");
	}

	if ((upper < 0) || (upper < lower))
	{
		fail("Invalid boundaries: %d to %d\n", lower, upper);
	}

	genpow2(lower, upper);
	genprime(lower, upper);

	return 0;
}


static void genpow2(int lower, int upper)
{
	for (int i = 2; i <= upper; i *= 2)
	{
		if (i >= lower)
		{
			printf("%d ", i); // emit po2
		}
	}
}

static void genprime(int lower, int upper)
{
	int* sieve = xmalloc((upper+1) * sizeof(int));
	memset(sieve, 0, (upper+1) * sizeof(int));

	if (lower < 3) lower = 3; // 2 is already in genpow2

	for(int i = 2; i <= upper; i++)
	{
		if (sieve[i]) continue;

		if (i >= lower)
		{
			printf("%d ", i); // emit prime
		}

		for (int j = i; j <= upper; j += i)
		{
			sieve[j] = 1;
		}
	}

	free(sieve);
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

static void fail(const char* format, ...)
{
	va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
	exit(1);
}
