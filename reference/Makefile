CFLAGS=-I../include --std=gnu99
UTIL_O=../include/util/util.o
UTILS=$(UTIL_O)

all: matmult stencil prefix

$(UTIL_O) : ../include/util/util.c
	gcc $(CFLAGS) -c $^ -o $@ -lrt

matmult: matmult.c $(UTILS)
	gcc $(CFLAGS) matmult.c $(UTIL_O) -o $@ -lrt

stencil: stencil.c $(UTILS)
	gcc $(CFLAGS) stencil.c $(UTIL_O) -o $@ -lrt

prefix: prefix-sums.c $(UTILS)
	gcc $(CFLAGS) prefix-sums.c $(UTIL_O) -o $@ -lrt
clean:
	rm matmult stencil prefix
	
.PHONY: all clean
