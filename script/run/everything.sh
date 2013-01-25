#!/bin/bash

SELF=$(basename $0)

# directory check
if [ ! -d script ]
then
	echo '[$SELF] WRONG DIRECTORY!' >&2
	exit 1
fi

script/run/make-all.sh

script/run/omp_p1.sh
script/run/omp_p2.sh

script/run/cilk.sh

script/run/mpi_p1.sh
script/run/mpi_p2.sh
script/run/mpi_p3.sh

echo 'ALL DONE'
