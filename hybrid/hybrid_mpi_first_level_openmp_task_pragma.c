#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <mpi.h>


pthread_mutex_t solutions_mutex;
uint64_t solutions = 0; // Shared variable to store the sum of solutions

// An abstract representation of an NxN chess board to tracking open positions
struct chess_board
{
    uint32_t n_size;           // Number of queens on the NxN chess board
    uint32_t *queen_positions; // Store queen positions on the board
    uint32_t *column;          // Store available column moves/attacks
    uint32_t *diagonal_up;     // Store available diagonal moves/attacks
    uint32_t *diagonal_down;
    uint32_t column_j;   // Stores column to place the next queen in
    uint64_t placements; // Tracks total number queen placements
    uint64_t solutions;  // Tracks number of solutions
    uint64_t start;
    uint64_t end;
};
struct chess_board *board;
struct chess_board *copyBoard(const struct chess_board *board);


// Handles dynamic memory allocation of the arrays and sets initial values
static void initialize_board(const uint32_t n_queens, struct chess_board **board, uint32_t start, uint32_t end)
{
    if (n_queens < 1)
    {
        fprintf(stderr, "The number of queens must be greater than 0.\n");
        exit(EXIT_SUCCESS);
    }

    // Dynamically allocate memory for chessboard struct
    *board = malloc(sizeof(struct chess_board));
    if (*board == NULL)
    {
        fprintf(stderr, "Memory allocation failed for chess board.\n");
        exit(EXIT_FAILURE);
    }

    // Dynamically allocate memory for chessboard arrays that track positions
    const uint32_t diagonal_size = 2 * n_queens - 1;
    const uint32_t total_size = 2 * (n_queens + diagonal_size);
    (*board)->queen_positions = malloc(sizeof(uint32_t) * total_size);
    if ((*board)->queen_positions == NULL)
    {
        fprintf(stderr, "Memory allocation failed for the chess board arrays.\n");
        free(*board);
        exit(EXIT_FAILURE);
    }
    (*board)->column = &(*board)->queen_positions[n_queens];
    (*board)->diagonal_up = &(*board)->column[n_queens];
    (*board)->diagonal_down = &(*board)->diagonal_up[diagonal_size];

    // Initialize the chess board parameters
    (*board)->n_size = n_queens;
    for (uint32_t i = 0; i < n_queens; ++i)
    {
        (*board)->queen_positions[i] = 0;
    }
    for (uint32_t i = n_queens; i < total_size; ++i)
    {
        // Initializes values for column, diagonal_up, and diagonal_down
        (*board)->queen_positions[i] = 1;
    }
    (*board)->column_j = 0;
    (*board)->placements = 0;
    (*board)->solutions = 0;
    (*board)->start = start;
    (*board)->end = end;
}
/*
// Frees the dynamically allocated memory for the chess board structure
static void smash_board(struct chess_board *board)
{
    free(board->queen_positions);
    free(board);
}
*/

// Check if a queen can be placed in at row 'i' of the current column
static uint32_t square_is_free(const uint32_t row_i, struct chess_board *board)
{
    return board->column[row_i] &
           board->diagonal_up[(board->n_size - 1) + (board->column_j - row_i)] &
           board->diagonal_down[board->column_j + row_i];
}

// Place a queen on the chess board at row 'i' of the current column
static void set_queen(const uint32_t row_i, struct chess_board *board)
{
    board->queen_positions[board->column_j] = row_i;
    board->column[row_i] = 0;
    board->diagonal_up[(board->n_size - 1) + (board->column_j - row_i)] = 0;
    board->diagonal_down[board->column_j + row_i] = 0;
    ++board->column_j;
    ++board->placements;
}

// Removes a queen from the NxN chess board in the given column to backtrack
static void remove_queen(const uint32_t row_i, struct chess_board *board)
{
    --board->column_j;
    board->diagonal_down[board->column_j + row_i] = 1;
    board->diagonal_up[(board->n_size - 1) + (board->column_j - row_i)] = 1;
    board->column[row_i] = 1;
}
/*
// Prints the number of queen placements and solutions for the NxN chess board
static void print_counts(struct chess_board *board)
{
    // The next line fixes double-counting when solving the 1-queen problem
    const uint64_t solution_count = board->n_size == 1 ? 1 : board->solutions;
    const char const output[] = "The %u-Queens problem required %lu queen "
                                "placements to find all %lu solutions\n";
    fprintf(stdout, output, board->n_size, board->placements, solution_count);
}
*/
void place_next_queen(const uint32_t row_boundary, struct chess_board *board)
{
    #pragma omp parallel
    #pragma omp single nowait
    {
        place_next_queen_parallel(row_boundary, board);
    }
    
}

void place_next_queen_parallel(const uint32_t row_boundary , struct chess_board *boardy)
{
    uint64_t start = boardy->start;
    uint64_t end = boardy->end;
    uint32_t n_size = boardy->n_size ;           // Number of queens on the NxN chess board

    const uint32_t middle = boardy->column_j ? boardy->n_size : boardy->n_size >> 1;

    for (uint32_t row_i = start; row_i < end; ++row_i)
    {      
        //printf("thread parent %d\n", omp_get_thread_num());
        uint32_t isfree = square_is_free(row_i, boardy);
        if (isfree)
        {
            #pragma omp task firstprivate(boardy)
            {       
                struct chess_board *local_board = copyBoard(boardy);
                set_queen(row_i, local_board);
                if (local_board->column_j == local_board->n_size)
                {
                    local_board->solutions += 2;
                }
                else
                {

                    if(local_board->column_j < local_board->n_size/4 + 1) {
                        if (local_board->queen_positions[0] != middle) {
                            local_board->start = 0;
                            local_board->end = local_board->n_size;
                            place_next_queen_parallel(local_board->n_size, local_board);
                        } else {
                            local_board->start = 0;
                            local_board->end = middle;
                            place_next_queen_parallel(middle, local_board);
                        }
                    } else {
                        if (local_board->queen_positions[0] != middle) {
                            local_board->start = 0;
                            local_board->end = local_board->n_size;
                            place_next_queen_seq(local_board->n_size, local_board);
                        } else {
                            local_board->start = 0;
                            local_board->end = middle;
                            place_next_queen_seq(middle, local_board);
                        }
                    }
                    
                }
                remove_queen(row_i, local_board);
            }

        }
    }
    #pragma omp taskwait
}
struct chess_board *copyBoard(const struct chess_board *board)

{
  struct chess_board *task = malloc(sizeof(struct chess_board));

  if (board->n_size < 1)
  {
    fprintf(stderr, "The number of queens must be greater than 0.\n");
    exit(EXIT_SUCCESS);
  }

  // Dynamically allocate memory for chessboard struct
  task = malloc(sizeof(struct chess_board));
  if (task == NULL)
  {
    fprintf(stderr, "Memory allocation failed for chess board.\n");
    exit(EXIT_FAILURE);
  }

  // Dynamically allocate memory for chessboard arrays that track positions
  const uint32_t diagonal_size = 2 * board->n_size - 1;
  const uint32_t total_size = 2 * (board->n_size + diagonal_size);
  (task)->queen_positions = malloc(sizeof(uint32_t) * total_size);
  if ((task)->queen_positions == NULL)
  {
    fprintf(stderr, "Memory allocation failed for the chess board arrays.\n");
    free(task);
    exit(EXIT_FAILURE);
  }
  (task)->column = &(task)->queen_positions[board->n_size];
  (task)->diagonal_up = &(task)->column[board->n_size];
  (task)->diagonal_down = &(task)->diagonal_up[diagonal_size];

  // Initialize the chess board parameters
  (task)->n_size = board->n_size;
  for (uint32_t i = 0; i < board->n_size; ++i)
  {
    task->queen_positions[i] = board->queen_positions[i];
  }
  for (uint32_t i = board->n_size; i < total_size; ++i)
  {
    // Initializes values for column, diagonal_up, and diagonal_down
    task->queen_positions[i] = board->queen_positions[i];
  }
  task->column_j = board->column_j;
  task->placements = board->placements;
  task->start = board->start;
  task->end = board->end;

  return task;
}



int tasks_created = 0;

void place_next_queen_seq(const uint32_t row_boundary, struct chess_board *boardy)
{
    uint64_t start = boardy->start;
    uint64_t end = boardy->end;
    //printf("seq version\n");
    const uint32_t middle = boardy->column_j ? boardy->n_size : boardy->n_size >> 1;

    for (uint32_t row_i = start; row_i < end; ++row_i)
    {
        if (square_is_free(row_i, boardy))
        {
            set_queen(row_i, boardy);
            if (boardy->column_j == boardy->n_size)
            {
                #pragma omp atomic 
                solutions += 2;
            }
            else
            {
                if (boardy->queen_positions[0] != middle)
                {
                    boardy->start = 0;
                    boardy->end = boardy->n_size;
                    place_next_queen_seq(boardy->n_size, boardy);
                }
                else
                {
                    boardy->start = 0;
                    boardy->end = middle;
                    place_next_queen_seq(middle, boardy);
                }
            }
            remove_queen(row_i, boardy);
        }
    }
}

int main(int argc, char *argv[])
{
    static const uint32_t default_n = 4;
    int num_procs = 2;
    int rank;

    const uint32_t n_queens = (argc != 1) ? (uint32_t)atoi(argv[1]) : default_n;
    clock_t start_time = clock();
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // initialize_board(n_queens,board);

    // Determines the index for the middle row to take advantage of board
    // symmetry when searching for solutions
    const uint32_t row_boundary = (n_queens >> 1) + (n_queens & 1);




        struct chess_board *board;
        uint32_t start = rank * row_boundary / num_procs;
        uint32_t end = (rank + 1) * row_boundary / num_procs;
        initialize_board(n_queens, &board, start, end);  
        omp_set_num_threads(4);
        place_next_queen(row_boundary,board);
        uint64_t total_solutions = 0;
        printf(" %lu solutions\n", solutions);
        MPI_Reduce(&solutions, &total_solutions, 1, MPI_UINT64_T, MPI_SUM, 0, MPI_COMM_WORLD);
        // Print the total number of solutions from the root process
        if (rank == 0) {
            clock_t end_time = clock();
            double time_totale = (double)(end_time-start_time)/CLOCKS_PER_SEC;
            printf("program takes : %f s \n",time_totale);
            printf("The %u-Queens problem has %lu solutions\n", n_queens, total_solutions);
        }

        // Clean up the MPI environment
        MPI_Finalize();
        return EXIT_SUCCESS;

    }



    

