#!/bin/bash

# Output for square matrices.
# $1: File Name
# $2: Number of threads
# $3: Output file
cat ./script/graph/omp_p2_fixed_threads.R | R --vanilla --slave --args $1 $2 $3
exit 0
