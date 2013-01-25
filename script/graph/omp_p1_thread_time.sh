#!/bin/bash

# $1: File Name
# $2: Desired computated length
# $3: Output File Name
cat omp_p1_thread_time.R | R --vanilla --slave --args $1 $2 $3
exit 0
