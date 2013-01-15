CFLAGS=-I../../include --std=c99 -fopenmp -Wall
UTIL_O=../../include/util/util.o

all: matmult

.utils: $(UTIL_O)
	true

$(UTIL_O) : ../../include/util/args.c
	gcc $(CFLAGS) -c $^ -o $@

matmult: matmult.c .utils
	gcc $(CFLAGS) matmult.c $(UTIL_O) -o $@

clean:
	rm matmult
	
.PHONY: all .utils clean
	