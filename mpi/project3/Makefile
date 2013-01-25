CFLAGS=-g --std=gnu99 -I../../include -D_XOPEN_SOURCE=600
UTIL_O=../../include/util/util.o

all: allgather reduce-scatter

.utils: $(UTIL_O)
	true

$(UTIL_O) : ../../include/util/args.c
	gcc $(CFLAGS) -c $^ -o $@

allgather: allgather.c .utils
	mpicc $(CFLAGS) allgather.c $(UTIL_O) -o $@

reduce-scatter: reduce-scatter.c .utils
	mpicc $(CFLAGS) reduce-scatter.c $(UTIL_O) -o $@

clean:
	rm allgather reduce-scatter

.PHONY: all .utils clean
