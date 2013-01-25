#!/bin/bash

SELF=$(basename $0)
RESULT_DIR='./result'
EXES='./mpi/project3/allgather ./mpi/project3/reduce-scatter'

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

numgen_m='./script/numgen/numgen 256 260'      # result size
numgen_n='./script/numgen/numgen 256 260'      # vector size
numgen_t='./script/numgen/numgen 1 5'          # thread count

for x in $EXES
do
	prog=$(basename $x)
	csv="$RESULT_DIR/$TIME-mpi-p2-$prog.csv"
	touch $csv

	for m in $($numgen_m)
	do

	for n in $($numgen_n)
	do

	for t in $($numgen_t)
	do

		echo "[$SELF] RUN $prog m=$m n=$n t=$t"
		mpirun -np $t $x $m $n 2>/dev/null >> $csv # || echo "FAIL"

	done

	done

	done
done

echo "[$SELF] DONE"
