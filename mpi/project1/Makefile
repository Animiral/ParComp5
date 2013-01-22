CFLAGS=--std=gnu99 -I../../include -D_XOPEN_SOURCE=600
UTIL_O=../../include/util/util.o

all: stencil

.utils: $(UTIL_O)
	true

$(UTIL_O) : ../../include/util/args.c
	gcc $(CFLAGS) -c $^ -o $@

stencil: stencil.c .utils
	mpicc $(CFLAGS) stencil.c $(UTIL_O) -o $@

clean:
	rm stencil

.PHONY: all .utils clean
