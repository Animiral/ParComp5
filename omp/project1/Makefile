CFLAGS=--std=c99 -fopenmp

all: recursive iterative hillis-steele totalsum

recursive: recursive.c
	gcc $(CFLAGS) $^ -o $@

iterative: iterative.c
	gcc $(CFLAGS) $^ -o $@

hillis-steele: hillis-steele.c
	gcc $(CFLAGS) $^ -o $@

totalsum: totalsum.c
	gcc $(CFLAGS) $^ -o $@

clean:
	rm recursive iterative hillis-steele totalsum
	