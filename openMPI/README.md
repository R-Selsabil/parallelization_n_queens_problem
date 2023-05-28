# parallelization_n_queens_problem
This repository contains the source code and results of a mini-project on high-performance computing, which aimed to propose and implement parallel solutions to the N-Queens problem using different parallel programming models, including PThreads, OpenMP, MPI, and a hybrid model using OpenMP/MPI. 

# OpenMPI
In the OpenMPI approach, we decided to parallelize the first level (first column of the board) since we are limited to using only two processes.