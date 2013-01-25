CFLAGS=-I../include -D_XOPEN_SOURCE=600
UTIL_O=../include/util/util.o
UTILS=$(UTIL_O)

all: prefix-sums

$(UTIL_O) : ../include/util/util.c
	gcc $(CFLAGS) -c $^ -o $@

prefix-sums: prefix-sums.cilk $(UTILS)
	cilkc $(CFLAGS) prefix-sums.cilk $(UTIL_O) -o $@

clean:
	rm prefix-sums

.PHONY: all clean
