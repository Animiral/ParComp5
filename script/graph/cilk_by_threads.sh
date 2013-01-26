#!/bin/bash

# $1: File Name
# $2: Length
# $3: Output File Name
cat ./script/graph/cilk_by_threads.R | R --vanilla --slave --args $1 $2 $3
exit 0
