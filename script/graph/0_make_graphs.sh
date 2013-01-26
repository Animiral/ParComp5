# Move old results to archives
#mkdir -p  ./archive
#mv ./result/* ./archive/


# Get new results
#scp -p 31989 $1@jupiter.par.tuwien.ac.at:result/* ./result/
#scp -p 31989 $1@saturn.par.tuwien.ac.at:result/* ./result/
# Run all graph scripts

# Set the target directory for graphs.
target="graphs"
mkdir -p graphs


# ### OMP P1 ###
# numgen_n='./script/numgen/numgen 100 500'       # input size
# numgen_t='./script/numgen/numgen 1 32'          # thread count
# recFile=$(ls ./result/ | grep -E *omp\-p1\-recursive.csv)
# iteFile=$(ls ./result/ | grep -E *omp\-p1\-iterative.csv)
# hilFile=$(ls ./result/ | grep -E *omp\-p1\-hillis\-steele.csv)
# totFile=$(ls ./result/ | grep -E *omp\-p1\-totalsum.csv)

# for n in $($numgen_n)
# do

# ./script/graph/omp_p1_thread_time.sh "./result/$recFile" "$n" "./$target/omp_p1_recursive_thread_$n.jpg"
# ./script/graph/omp_p1_thread_time.sh "./result/$iteFile" "$n" "./$target/omp_p1_iterative_thread_$n.jpg"
# ./script/graph/omp_p1_thread_time.sh "./result/$hilFile" "$n" "./$target/omp_p1_hillis_thread_$n.jpg"
# #./script/graph/omp_p1_thread_time.sh "./result/$totFile" "$n" "./$target/omp_p1_total_thread_$n.jpg"

# #./script/graph/omp_p1_compare.sh "./result/$recFile" "./result/$iteFile" "./result/$hilFile" "./result/$totFile" "$n" "./$target/omp_p1_compare_$n.jpg"

# done

# for t in $($numgen_t)
# do
# 	./script/graph/omp_p1_length_time.sh ./result/*omp-p1-recursive.csv "$t" "./$target/omp_p1_recursive_length_$t.jpg"
# 	./script/graph/omp_p1_length_time.sh ./result/*omp-p1-iterative.csv "$t" "./$target/omp_p1_iterative_length_$t.jpg"
# 	./script/graph/omp_p1_length_time.sh ./result/*omp-p1-hillis-steele.csv "$t" "./$target/omp_p1_hillis_length_$t.jpg"
# 	#./script/graph/omp_p1_length_time.sh ./result/*omp-p1-totalsum.csv "$t" "./$target/omp_p1_total_length_$t.jpg"

# done


# ### OMP P2 ###
# numgen_m='./script/numgen/numgen 10 70'       # input size m
# numgen_n='./script/numgen/numgen 10 70'       # input size n
# numgen_t='./script/numgen/numgen 1 32'          # thread count


# omp2File=$(ls ./result/ | grep -E *omp\-p2-matmult.csv)
# for m in $($numgen_m)
# do

# for n in $($numgen_n)
# do

# ./script/graph/omp_p2_fixed_size.sh "./result/$omp2File" "$m" "$n" "$target/omp_p2_fixed_size_$m_$n.jpg"


# done

# done



# for t in $($numgen_t)
# do

# ./script/graph/omp_p2_fixed_threads.sh "./result/$p2File" "$t" "$target/omp_p2_fixed_threads_$t.jpg"

# done


### CILK ###

numgen_n='./script/numgen/numgen 100 500'       # input size
numgen_t='./script/numgen/numgen 1 32'          # thread count

cilkFile=$(ls ./result/ | grep -E *cilk\-prefix\-sums.csv)

for n in $($numgen_n)
do
	./script/graph/cilk_by_threads.sh ./result/cilk.csv "$n" "$target/cilk_by_threads_$n.jpg"

done

for t in $($numgen_t)
do
	./script/graph/cilk_fixed_threads.sh ./result/cilk.csv "$t" "$target/cilk_fixed_threads_$t.jpg" 
	
done






### MPI Project 1 ###
numgen_m='./script/numgen/numgen 256 256'      # input rows
numgen_n='./script/numgen/numgen 256 256'      # input columns
numgen_c='./script/numgen/numgen 1 5'          # nr of block columns
numgen_t='./script/numgen/numgen 1 5'          # thread count

mpi1File=$(ls ./result/ | grep -E *mpi\-p1\-stencil.csv)

for t in $($numgen_t)
do

	./script/graph/mpi_p1_vary_total_size.sh "./result/$mpi1File" "$t" "$target/mpi_p1_vary_size_$t.jpg"

	for m in $($numgen_m)
	do

	for n in $($numgen_n)
	do

		./script/graph/mpi_p1_vary_process_distribution.sh "./result/$mpi1File" "$m" "$n" "$t" "$target/mpi_pi1_vary_process_$m_$n_$t.jpg"

	done

	done

done





### MPI Project 2 ###
mpi2File=$(ls ./result/ | grep -E *mpi\-p2\-prefix\-sums.csv)

./script/graph/mpi_p2.sh "./result/$mpi2aFile" "$target/mpi_p2-prefix-sums.jpg"



### MPI Project 3 ###

numgen_m='./script/numgen/numgen 256 260'      # result size
numgen_n='./script/numgen/numgen 256 260'      # vector size
numgen_t='./script/numgen/numgen 1 5'          # thread count

mpi3aFile=$(ls ./result/ | grep -E *mpi\-p3\-allgather.csv)\
mpi3bFile=$(ls ./result/ | grep -E *mpi\-p3\-reduce-scatter.csv)

        for m in $($numgen_m)
        do

        for n in $($numgen_n)
        do
        	.script/graph/mpi_p3.sh "./result/$mpi3aFile" "./result/$mpi3bFile" "$m" "n" "$target/mpi_p3_$m_$n.jpg"
        done

        done