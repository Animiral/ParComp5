#!/bin/bash

SELF=$(basename $0)
RESULT_DIR='./result'
EXES='./omp/project1/recursive ./omp/project1/iterative ./omp/project1/hillis-steele ./omp/project1/totalsum'

# directory check
if [ ! -d script ]
then
	echo '[$SELF] WRONG DIRECTORY!' >&2
	exit 1
fi

# timestamp or other run-identifying parameter
if [ $1 ]
then
	TIME=$1
else
	TIME=$(date +'%m-%d-%H%M-%S')
fi

mkdir -p $RESULT_DIR

numgen_n='./script/numgen/numgen 1 250000000'   # input size
numgen_t='./script/numgen/numgen 1 2048'          # thread count

for x in $EXES
do
	prog=$(basename $x)
	csv="$RESULT_DIR/$TIME-omp-p1-$prog.csv"
	touch $csv

	for n in $($numgen_n)
	do

	for t in $($numgen_t)
	do

		echo "[$SELF] RUN $prog n=$n t=$t"
		$x $n $t >> $csv # || echo "FAIL"

	done

	done
done

echo "[$SELF] DONE"
