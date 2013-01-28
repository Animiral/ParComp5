CFLAGS=-g --std=gnu99 -I../../include -D_XOPEN_SOURCE=600
UTIL_O=../../include/util/util.o
UTILS=$(UTIL_O)

all: numgen numgen2 numgen3

$(UTIL_O) : ../../include/util/util.c
	gcc $(CFLAGS) -c $^ -o $@

numgen: numgen.c $(UTILS)
	mpicc $(CFLAGS) numgen.c $(UTIL_O) -o $@

numgen2: numgen.c $(UTILS)
	mpicc $(CFLAGS) -DNOPRIMES numgen.c $(UTIL_O) -o $@

numgen3: numgen.c $(UTILS)
	mpicc $(CFLAGS) -DLESSPRIMES numgen.c $(UTIL_O) -o $@

clean:
	rm numgen numgen2 numgen3

.PHONY: all clean
