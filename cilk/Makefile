CFLAGS=-I../include -D_XOPEN_SOURCE=600
UTIL_O=../include/util/util.o

all: prefix-sums

.utils: $(UTIL_O)
	true

$(UTIL_O) : ../include/util/args.c
	gcc $(CFLAGS) -c $^ -o $@

prefix-sums: prefix-sums.cilk .utils
	cilkc $(CFLAGS) prefix-sums.cilk $(UTIL_O) -o $@

clean:
	rm prefix-sums

.PHONY: all .utils clean
