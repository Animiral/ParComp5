CFLAGS=-I../../include --std=c99 -fopenmp
UTIL_O=../../include/util/util.o
UTILS=$(UTIL_O)

all: recursive iterative hillis-steele totalsum

$(UTIL_O) : ../../include/util/util.c
	gcc $(CFLAGS) -c $^ -o $@

recursive: recursive.c $(UTILS)
	gcc $(CFLAGS) recursive.c $(UTIL_O) -o $@

iterative: iterative.c $(UTILS)
	gcc $(CFLAGS) iterative.c $(UTIL_O) -o $@

hillis-steele: hillis-steele.c $(UTILS)
	gcc $(CFLAGS) hillis-steele.c $(UTIL_O) -o $@

totalsum: totalsum.c $(UTILS)
	gcc $(CFLAGS) totalsum.c $(UTIL_O) -o $@

clean:
	rm recursive iterative hillis-steele totalsum
	
.PHONY: all clean
