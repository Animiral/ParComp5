#include <omp.h>

int main()
{
	int i;
	int a[1024];

	#pragma omp parallel for
	for (i = 0; i < 1024; i++)
	{
		a[i] = omp_get_thread_num();
	}

	for (i = 0; i < 1024; i++)
	{
		printf("%d,", a[i]);
	}
}
