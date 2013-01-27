#!/bin/bash

SELF=$(basename $0)
RESULT_DIR='./result'
EXES='./reference/stencil ./reference/prefix ./reference/matmult'


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

numgen_m='./script/numgen/numgen 100 500'       # input size
numgen_n='./script/numgen/numgen 100 300'       # vector size

csv_stencil="$RESULT_DIR/$TIME-reference-stencil.csv"
csv_prefix="$RESULT_DIR/$TIME-reference-prefix.csv"
csv_matmult="$RESULT_DIR/$TIME-reference-matmult.csv"

touch $csv_stencil
touch $csv_prefix
touch $csv_matmult

for m in $($numgen_m)
do

for n in $($numgen_n)
do
	echo "[$SELF] RUN prefix n=$m"
	./reference/prefix $m >> $csv_prefix

	echo "[$SELF] RUN matmult m=$m n=$n"
	./reference/matmult $m $n >> $csv_matmult

	echo "[$SELF] RUN stencil m=$m n=$n"
	./reference/stencil $m $n >> $csv_stencil


done

done

echo "[$SELF] DONE"
