
# Get all results from server

# Run all graph scripts

i=2
./omp_p1_length_time.sh ../../result/omp-p1-recursive.csv "$i" "graphs/omp_p1_recursive_length_$i.jpg"
./omp_p1_length_time.sh ../../result/omp-p1-iterative.csv "$i" "graphs/omp_p1_iterative_length_$i.jpg"
./omp_p1_length_time.sh ../../result/omp-p1-hillis-steele.csv "$i" "graphs/omp_p1_hillis_length_$i.jpg"
./omp_p1_length_time.sh ../../result/omp-p1-totalsum.csv "$i" "graphs/omp_p1_total_length_$i.jpg"

j=100
./omp_p1_thread_time.sh ../../result/omp-p1-recursive.csv "$j" "graphs/omp_p1_recursive_thread_$j.jpg"
./omp_p1_thread_time.sh ../../result/omp-p1-iterative.csv "$j" "graphs/omp_p1_iterative_thread_$j.jpg"
./omp_p1_thread_time.sh ../../result/omp-p1-hillis-steele.csv "$j" "graphs/omp_p1_hillis_thread_$j.jpg"
./omp_p1_thread_time.sh ../../result/omp-p1-totalsum.csv "$j" "graphs/omp_p1_total_thread_$j.jpg"


./omp_p1_compare.sh ../../result/omp-p1-recursive.csv ../../result/omp-p1-iterative.csv ../../result/omp-p1-hillis-steele.csv ../../result/omp-p1-totalsum.csv "$j" "graphs/omp_p1_compare_$j.jpg"

n=320
m=80
t=8
./omp_p2_fixed_size.sh ../../result/omp-p2.csv "$n" "$m" "graphs/omp_p2_fixed_size_$n_$m.jpg"

./omp_p2_fixed_threads.sh ../../result/omp-p2.csv "$t" "graphs/omp_p2_fixed_threads_$t.jpg"


thrds=1
len=16
./cilk_fixed_threads.sh ../../result/cilk.csv "$thrds" "graphs/cilk_fixed_threads_$thrds.jpg" 

./cilk_by_threads.sh ../../result/cilk.csv "$len" "graphs/cilk_by_threads_$len.jpg"
