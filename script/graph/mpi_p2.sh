#!bin/bash

# (n, p, time) -- Pass all arguments

cat mpi_p2.R | R --vanilla --slave --args $1 $2