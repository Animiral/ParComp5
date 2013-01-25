#!/bin/bash

# directory check
if [ ! -d script ]
then
	echo 'WRONG DIRECTORY!' >&2
	exit 1
fi

RESULT_DIR='./result'
TIME=$(date +'%m-%d-%H%M-%S')

mkdir -p $RESULT_DIR

# input size
N_MIN=100
N_MAX=1000000 # for demo; increase to several GB later
N_INC=2 # mult factor

# chunk size
C_MIN=1
C_INC=2 # mult factor

EXES='./cilk/prefix-sums'

for x in $EXES
do
	prog=$(basename $x)
	csv="$RESULT_DIR/$TIME-cilk-$prog.csv"
	touch $csv

	n=$N_MIN
	while [ $n -le $N_MAX ]
	do
		echo "RUN $prog n=$n"

		let C_MAX=$n/2
		c=$C_MIN
		while [ $c -le $C_MAX ]
		do
			# TODO: number of threads/nodes
			# use Cilk_active_size, Self from <cilk.h>
			echo "$x --nproc 1 $n 1 $c"
			$x --nproc 1 $n 1 $c >> $csv # || echo "FAIL"

			let c=$c*$C_INC;
		done

		let n=$n*$N_INC;
	done
done

echo "DONE"
