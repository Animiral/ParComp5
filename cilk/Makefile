CFLAGS=-D_XOPEN_SOURCE=600

all: prefix-sums

prefix-sums: prefix-sums.cilk
	cilkc $(CFLAGS) $^ -o $@

clean:
	rm prefix-sums
