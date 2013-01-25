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

# input size n
N_MIN=10
N_MAX=1024 # for demo; increase to several GB later
N_INC=2 # mult factor

# input size m
M_MIN=10
M_MAX=1024 # for demo; increase to several GB later
M_INC=2 # mult factor

# thread count
T_MIN=1
T_MAX=2048
T_INC=2 # mult factor

EXES='./omp/project2/matmult'

for x in $EXES
do
	prog=$(basename $x)
	csv="$RESULT_DIR/$TIME-omp-p2-$prog.csv"
	touch $csv

	m=$M_MIN
	while [ $m -le $M_MAX ]
	do
		n=$N_MIN
		while [ $n -le $N_MAX ]
		do
			echo "RUN $prog m=$m n=$n"

			t=$T_MIN
			while [ $t -le $T_MAX ]
			do
				echo "$x $m $n $t"
				$x $m $n $t >> $csv # || echo "FAIL"

				let t=$t*$T_INC;
			done

			let n=$n*$N_INC;
		done

		let m=$m*$M_INC;
	done
done

echo "DONE"
