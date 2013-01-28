#!/bin/bash

SELF=$(basename $0)
RESULT_DIR='./result'
EXES='./cilk/prefix-sums'

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

numgen_n='./script/numgen/numgen 1 250000'       # input size
numgen_c='./script/numgen/numgen 1 30000'          # chunk size
numgen_t='./script/numgen/numgen 1 2048'          # thread count

for x in $EXES
do
	prog=$(basename $x)
	csv="$RESULT_DIR/$TIME-cilk-$prog.csv"
	touch $csv

	for n in $($numgen_n)
	do

	for c in $($numgen_c)
	do

	for t in $($numgen_t)
	do

		echo "[$SELF] RUN $prog n=$n c=$c t=$t"
		# TODO: number of threads/nodes
		# use Cilk_active_size, Self from <cilk.h>
		$x --nproc $t $n $c >> $csv # || echo "FAIL"

	done

	done

	done
done

echo "[$SELF] DONE"
