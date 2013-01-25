#!/bin/bash

# directory check
if [ ! -d script ]
then
	echo 'WRONG DIRECTORY!' >&2
	exit 1
fi

pushd ./script/numgen && make; popd
pushd ./omp/project1 && make; popd
pushd ./omp/project2 && make; popd
pushd ./cilk && make; popd
pushd ./mpi/project1 && make; popd
pushd ./mpi/project2 && make; popd
pushd ./mpi/project3 && make; popd
