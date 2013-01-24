#!/bin/bash

# $1: File Name
# $2: n Size of matrix
# $3: m Size of matrix
# $4: Output File Name
cat omp_p2_fixed_size.sh.R | R --vanilla --slave --args $1 $2 $3 $4
exit 0
