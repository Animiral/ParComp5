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

numgen_p='./script/numgen/numgen 1 250000000'   # prefix input size
numgen_m='./script/numgen/numgen 1 30000'       # input size
numgen_n='./script/numgen/numgen 1 30000'       # vector size

csv_stencil="$RESULT_DIR/$TIME-reference-stencil.csv"
csv_prefix="$RESULT_DIR/$TIME-reference-prefix.csv"
csv_matmult="$RESULT_DIR/$TIME-reference-matmult.csv"

touch $csv_stencil
touch $csv_prefix
touch $csv_matmult

for p in $($numgen_p)
do
	echo "[$SELF] RUN prefix n=$p"
	./reference/prefix $p >> $csv_prefix

done


for m in $($numgen_m)
do

for n in $($numgen_n)
do
	echo "[$SELF] RUN matmult m=$m n=$n"
	./reference/matmult $m $n >> $csv_matmult

	echo "[$SELF] RUN stencil m=$m n=$n"
	./reference/stencil $m $n >> $csv_stencil


done

done

echo "[$SELF] DONE"
