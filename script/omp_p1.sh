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

# thread count
T_MIN=1
T_MAX=2048
T_INC=2 # mult factor

EXES='./omp/project1/recursive ./omp/project1/iterative ./omp/project1/hillis-steele ./omp/project1/totalsum'

for x in $EXES
do
	prog=$(basename $x)
	csv="$RESULT_DIR/$TIME-omp-p1-$prog.csv"
	touch $csv

	n=$N_MIN
	while [ $n -le $N_MAX ]
	do
		echo "RUN $prog n=$n"

		t=$T_MIN
		while [ $t -le $T_MAX ]
		do
			echo "$x $n $t"
			$x $n $t >> $csv # || echo "FAIL"

			let t=$t*$T_INC;
		done

		let n=$n*$N_INC;
	done
done

echo "DONE"
