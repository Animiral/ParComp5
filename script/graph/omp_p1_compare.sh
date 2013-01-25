#!/bin/bash

# $1: Recursive
# $2: Iterative
# $3: Hillis-Steele
# $4: Totalsum
# $5: Length
# $6: Output File Name
cat omp_p1_compare.R | R --vanilla --slave --args $1 $2 $3 $4 $5 $6
exit 0
