#!/bin/bash

SELF=$(basename $0)
RESULT_DIR='./result'
EXES='./mpi/project1/stencil'

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

numgen_m='./script/numgen/numgen 1 30000'      # input rows
numgen_n='./script/numgen/numgen 1 30000'      # input columns
numgen_c='./script/numgen/numgen 1 30000'          # nr of block columns
numgen_t='./script/numgen/numgen 1 2048'          # thread count

for x in $EXES
do
	prog=$(basename $x)
	csv="$RESULT_DIR/$TIME-mpi-p1-$prog.csv"
	touch $csv

	for m in $($numgen_m)
	do

	for n in $($numgen_n)
	do

	for c in $($numgen_c)
	do
		
	for t in $($numgen_t)
	do

		echo "[$SELF] RUN $prog m=$m n=$n c=$c t=$t"
		mpirun -np $t $x $m $n $c 2>/dev/null >> $csv # || echo "FAIL"

	done

	done

	done

	done
done

echo "[$SELF] DONE"
