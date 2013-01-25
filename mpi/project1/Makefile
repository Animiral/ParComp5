CFLAGS=--std=gnu99 -I../../include -D_XOPEN_SOURCE=600
UTIL_O=../../include/util/util.o
UTILS=$(UTIL_O)

all: stencil

$(UTIL_O) : ../../include/util/util.c
	gcc $(CFLAGS) -c $^ -o $@

stencil: stencil.c $(UTILS)
	mpicc $(CFLAGS) stencil.c $(UTIL_O) -o $@

clean:
	rm stencil

.PHONY: all clean
