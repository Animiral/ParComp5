CFLAGS=-I../../include --std=gnu99 -fopenmp -Wall
UTIL_O=../../include/util/util.o
UTILS=$(UTIL_O)

all: matmult

$(UTIL_O) : ../../include/util/util.c
	gcc $(CFLAGS) -c $^ -o $@

matmult: matmult.c $(UTILS)
	gcc $(CFLAGS) matmult.c $(UTIL_O) -o $@

clean:
	rm matmult
	
.PHONY: all clean
	