# parallelization_n_queens_problem
This repository contains the source code and results of a mini-project on high-performance computing, which aimed to propose and implement parallel solutions to the N-Queens problem using different parallel programming models, including PThreads, OpenMP, MPI, and a hybrid model using OpenMP/MPI. 


make : 
gcc -c n_queens_counter_lib.c -o n_queens_counter_lib.o
gcc openmp_tasking.c n_queens_counter_lib.o -o openmp_tasking -fopenmp
