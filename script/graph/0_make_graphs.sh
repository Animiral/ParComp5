# Get all results from server
#scp -p 31989 $1@jupiter.par.tuwien.ac.at:result/* ../../result/
#scp -p 31989 $1@saturn.par.tuwien.ac.at:result/* ../../result/
# Run all graph scripts
target="graphs"
i=2
./omp_p1_length_time.sh ../../result/omp-p1-recursive.csv "$i" "$target/omp_p1_recursive_length_$i.jpg"
./omp_p1_length_time.sh ../../result/omp-p1-iterative.csv "$i" "$target/omp_p1_iterative_length_$i.jpg"
./omp_p1_length_time.sh ../../result/omp-p1-hillis-steele.csv "$i" "$target/omp_p1_hillis_length_$i.jpg"
#./omp_p1_length_time.sh ../../result/omp-p1-totalsum.csv "$i" "$target/omp_p1_total_length_$i.jpg"

j=100
./omp_p1_thread_time.sh ../../result/omp-p1-recursive.csv "$j" "$target/omp_p1_recursive_thread_$j.jpg"
./omp_p1_thread_time.sh ../../result/omp-p1-iterative.csv "$j" "$target/omp_p1_iterative_thread_$j.jpg"
./omp_p1_thread_time.sh ../../result/omp-p1-hillis-steele.csv "$j" "$target/omp_p1_hillis_thread_$j.jpg"
#./omp_p1_thread_time.sh ../../result/omp-p1-totalsum.csv "$j" "$target/omp_p1_total_thread_$j.jpg"


./omp_p1_compare.sh ../../result/omp-p1-recursive.csv ../../result/omp-p1-iterative.csv ../../result/omp-p1-hillis-steele.csv ../../result/omp-p1-totalsum.csv "$j" "$target/omp_p1_compare_$j.jpg"

n=320
m=80
t=8
./omp_p2_fixed_size.sh ../../result/omp-p2.csv "$n" "$m" "$target/omp_p2_fixed_size_$n_$m.jpg"

./omp_p2_fixed_threads.sh ../../result/omp-p2.csv "$t" "$target/omp_p2_fixed_threads_$t.jpg"


thrds=1
len=400
./cilk_fixed_threads.sh ../../result/cilk.csv "$thrds" "$target/cilk_fixed_threads_$thrds.jpg" 

./cilk_by_threads.sh ../../result/cilk.csv "$len" "$target/cilk_by_threads_$len.jpg"


m=19
n=12
p=121

#./mpi_p1_vary_process_distribution.sh ../../result/mpi-p1.csv "$m" "$n" "$p" "$target/mpi_pi1_vary_process_$m_$n_$p.jpg"

#./mpi_p1_vary_total_size.sh ../../result/mpi-p1.csv "$p" "$target/mpi_pi1_vary_size_$p.jpg"

#./mpi_p2.sh ../../result/mpi-p2.csv "$target/mpi_p2.jpg"

#./mpi_p3.sh ../../result/mpi-p3-scatter.csv ../../result/mpi-p3-gather.csv "$m" "n" "$target/mpi_p3_$m_$n.jpg"
