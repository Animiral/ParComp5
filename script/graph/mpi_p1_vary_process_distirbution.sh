#!/bin/bash
# (m, n, r, c, p, time, ref_time)
# $1: File Name
# $2: m Rows
# $3: n Columns
# $4: p Processes
# $5: Output File Name
cat mpi_p1_vary_process_distribution.R | R --vanilla --slave --args $1 $2 $3 $4 $5
exit 0
