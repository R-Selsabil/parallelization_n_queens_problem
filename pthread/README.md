# parallelization_n_queens_problem
This repository contains the source code and results of a mini-project on high-performance computing, which aimed to propose and implement parallel solutions to the N-Queens problem using different parallel programming models, including PThreads, OpenMP, MPI, and a hybrid model using OpenMP/MPI.

# pthread
The initial idea was to parallelize the first column of the board, in order to parallilize more than one level, we thought of using a pool of tasks created by the main thread and can be executed in parallel.

make: 
gcc -pthread 