#!/bin/bash

# $1: File Name
# $2: Number of threads used
# $3: Output File Name
cat ./script/graph/omp_p1_length_time.R | R --vanilla --slave --args $1 $2 $3
exit 0
