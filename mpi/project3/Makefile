CFLAGS=-g --std=gnu99 -I../../include -D_XOPEN_SOURCE=600
UTIL_O=../../include/util/util.o
UTILS=$(UTIL_O)

all: allgather reduce-scatter

$(UTIL_O) : ../../include/util/util.c
	gcc $(CFLAGS) -c $^ -o $@

allgather: allgather.c $(UTILS)
	mpicc $(CFLAGS) allgather.c $(UTIL_O) -o $@

reduce-scatter: reduce-scatter.c $(UTILS)
	mpicc $(CFLAGS) reduce-scatter.c $(UTIL_O) -o $@

clean:
	rm allgather reduce-scatter

.PHONY: all clean
