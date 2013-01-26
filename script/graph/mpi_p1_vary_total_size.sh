#!/bin/bash
# (m, n, r, c, p, time, ref_time)
# $1: File Name
# $2: p Processes
# $3: Output File Name
cat ./script/graph/mpi_p1_vary_total_size.R | R --vanilla --slave --args $1 $2 $3
exit 0
