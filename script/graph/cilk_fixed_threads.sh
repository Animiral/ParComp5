#!/bin/bash

# $1: File Name
# $2: Threads
# $3: Output File Name
cat cilk_fixed_threads.R | R --vanilla --slave --args $1 $2 $3
exit 0
