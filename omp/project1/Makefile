CFLAGS=-I../../include --std=c99 -fopenmp
UTIL_O=../../include/util/util.o

all: recursive iterative hillis-steele totalsum

.utils: $(UTIL_O)
	true

$(UTIL_O) : ../../include/util/args.c
	gcc $(CFLAGS) -c $^ -o $@

recursive: recursive.c .utils
	gcc $(CFLAGS) recursive.c $(UTIL_O) -o $@

iterative: iterative.c .utils
	gcc $(CFLAGS) iterative.c $(UTIL_O) -o $@

hillis-steele: hillis-steele.c .utils
	gcc $(CFLAGS) hillis-steele.c $(UTIL_O) -o $@

totalsum: totalsum.c .utils
	gcc $(CFLAGS) totalsum.c $(UTIL_O) -o $@

clean:
	rm recursive iterative hillis-steele totalsum
	
.PHONY: all .utils clean
