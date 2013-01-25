#!bin/bash

# (n, m, p, time) -- Pass all arguments
# $1 Input File 1 (Reduce Scatter)
# $2 Input File 2 (All Gather)
# $3 Fixed m
# $4 Fixed n
# $5 Output file
cat mpi_p3.R | R --vanilla --slave --args $1 $2 $3 $4 $5