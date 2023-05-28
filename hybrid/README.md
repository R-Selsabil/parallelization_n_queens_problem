# parallelization_n_queens_problem
This repository contains the source code and results of a mini-project on high-performance computing, which aimed to propose and implement parallel solutions to the N-Queens problem using different parallel programming models, including PThreads, OpenMP, MPI, and a hybrid model using OpenMP/MPI. 


# hybrid
The idea behind this solution is to utilize OpenMPI to parallelize the first column, while employing OpenMP tasking to parallelize subsequent levels (columns) of the chess board.




