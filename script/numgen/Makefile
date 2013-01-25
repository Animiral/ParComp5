CFLAGS=-g --std=gnu99 -I../../include -D_XOPEN_SOURCE=600
UTIL_O=../../include/util/util.o
UTILS=$(UTIL_O)

all: numgen

$(UTIL_O) : ../../include/util/util.c
	gcc $(CFLAGS) -c $^ -o $@

numgen: numgen.c $(UTILS)
	mpicc $(CFLAGS) numgen.c $(UTIL_O) -o $@

clean:
	rm numgen

.PHONY: all clean
