#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * Argument parsing, up to 4 integers + optional debug flag as 1st arg
 * returns 0 on success, 1 on error
 */
int parse_args(int argc, char* argv[], int* debug_flag,
	const char* aname, int* a, 
	const char* bname, int* b, 
	const char* cname, int* c, 
	const char* dname, int* d)
{
	int i;
	int want;
	int have;

	*debug_flag = (argc > 1) && ((strcmp(argv[1], "--debug") == 0) || (strcmp(argv[1], "-d") == 0));

	want = (a != NULL) + (b != NULL) + (c != NULL) + (d != NULL);
	have = argc-1-*debug_flag;
	if (want != have)
	{
		fprintf(stderr, "Got %d arguments instead of the expected %d.\n", have, want);
		fprintf(stderr, "Optional arg: --debug\n");

		i = 1;
		if (a != NULL) fprintf(stderr, "Arg %d: <%s>\n", i++, aname);
		if (b != NULL) fprintf(stderr, "Arg %d: <%s>\n", i++, bname);
		if (c != NULL) fprintf(stderr, "Arg %d: <%s>\n", i++, cname);
		if (d != NULL) fprintf(stderr, "Arg %d: <%s>\n", i++, dname);

		return 1;
	}

	i = 1+*debug_flag;

	if (a != NULL) *a = strtol(argv[i++], NULL, 0);
	if (b != NULL) *b = strtol(argv[i++], NULL, 0);
	if (c != NULL) *c = strtol(argv[i++], NULL, 0);
	if (d != NULL) *d = strtol(argv[i++], NULL, 0);

	return 0;
}

