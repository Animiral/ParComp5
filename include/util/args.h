/*
 * Argument parsing, up to 4 integers + optional debug flag as 1st arg
 * returns 0 on success, 1 on error
 */
int parse_args(int argc, char* argv[], int* debug_flag,
	const char* aname, int* a, 
	const char* bname, int* b, 
	const char* cname, int* c, 
	const char* dname, int* d);
