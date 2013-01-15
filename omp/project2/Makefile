CFLAGS=--std=c99 -fopenmp -Wall

all: matmult

matmult: matmult.c
	gcc $(CFLAGS) $^ -o $@

clean:
	rm matmult
	