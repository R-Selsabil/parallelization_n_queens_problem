
// N-Queens Placement and Solutions Counter
// Author: James Walker
// Copyrighted 2017 under the MIT license:
//   http://www.opensource.org/licenses/mit-license.php
//
// Purpose:
//   The N-Queens Counter follows an improved version of the algorithm used by
//   the N-Queens Solver, except it does not return any of the solutions.
//   Instead, the program counts the number of solutions for a given N-Queens
//   problem as well as the number of times a queen is placed during the
//   program's execution.
// Compilation, Execution, and Example Output:
//   $ gcc -std=c99 -O2 n_queens_counter.c -o n_queens_counter
//   $ ./n_queens_counter.exe 12
//   The 12-Queens problem required 428094 queen placements to find all 14200
//   solutions
//
// This implementation was adapted from the algorithm provided at the bottom of
// this webpage:
//   www.cs.utexas.edu/users/EWD/transcriptions/EWD03xx/EWD316.9.html

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

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
};

// Handles dynamic memory allocation of the arrays and sets initial values
static void initialize_board(const uint32_t n_queens, struct chess_board **boardy)
{
    if (n_queens < 1)
    {
        fprintf(stderr, "The number of queens must be greater than 0.\n");
        exit(EXIT_SUCCESS);
    }

    // Dynamically allocate memory for chessboard struct
    *boardy = malloc(sizeof(struct chess_board));
    if (*boardy == NULL)
    {
        fprintf(stderr, "Memory allocation failed for chess board.\n");
        exit(EXIT_FAILURE);
    }

    // Dynamically allocate memory for chessboard arrays that track positions
    const uint32_t diagonal_size = 2 * n_queens - 1;
    const uint32_t total_size = 2 * (n_queens + diagonal_size);
    (*boardy)->queen_positions = malloc(sizeof(uint32_t) * total_size);
    if ((*boardy)->queen_positions == NULL)
    {
        fprintf(stderr, "Memory allocation failed for the chess board arrays.\n");
        free(*boardy);
        exit(EXIT_FAILURE);
    }
    (*boardy)->column = &(*boardy)->queen_positions[n_queens];
    (*boardy)->diagonal_up = &(*boardy)->column[n_queens];
    (*boardy)->diagonal_down = &(*boardy)->diagonal_up[diagonal_size];

    // Initialize the chess board parameters
    (*boardy)->n_size = n_queens;

    for (uint32_t i = 0; i < n_queens; ++i)
    {
        (*boardy)->queen_positions[i] = 0;
    }
    for (uint32_t i = n_queens; i < total_size; ++i)
    {
        // Initializes values for column, diagonal_up, and diagonal_down
        (*boardy)->queen_positions[i] = 1;
    }
    (*boardy)->column_j = 0;
    (*boardy)->placements = 0;
    (*boardy)->solutions = 0;
}

// Frees the dynamically allocated memory for the chess board structure
static void smash_board(struct chess_board *boardy)
{
    free(boardy->queen_positions);
    free(boardy);
}

// Check if a queen can be placed in at row 'i' of the current column
static uint32_t square_is_free(const uint32_t row_i, struct chess_board *boardy)
{

    return boardy->column[row_i] &
           boardy->diagonal_up[(boardy->n_size - 1) + (boardy->column_j - row_i)] &
           boardy->diagonal_down[boardy->column_j + row_i];
}

// Place a queen on the chess board at row 'i' of the current column
static void set_queen(const uint32_t row_i, struct chess_board *boardy)
{
    boardy->queen_positions[boardy->column_j] = row_i;
    boardy->column[row_i] = 0;
    boardy->diagonal_up[(boardy->n_size - 1) + (boardy->column_j - row_i)] = 0;
    boardy->diagonal_down[boardy->column_j + row_i] = 0;
    ++boardy->column_j;
    ++boardy->placements;
}

// Removes a queen from the NxN chess board in the given column to backtrack
static void remove_queen(const uint32_t row_i, struct chess_board *boardy)
{
    --boardy->column_j;
    boardy->diagonal_down[boardy->column_j + row_i] = 1;
    boardy->diagonal_up[(boardy->n_size - 1) + (boardy->column_j - row_i)] = 1;
    boardy->column[row_i] = 1;
}

void printQueenPositions(struct chess_board *board)
{
    printf("positions ");
    for (uint32_t i = 0; i < board->n_size; i++)
    {
        printf("%d ", board->queen_positions[i]);
    }
    printf("\n");

    printf("Column: ");
    for (uint32_t i = 0; i < board->n_size; i++)
    {
        printf("%d ", board->column[i]);
    }
    printf("\n");

    printf("Diagonal Up: ");
    for (uint32_t i = 0; i < (2 * board->n_size - 1); i++)
    {
        printf("%d ", board->diagonal_up[i]);
    }
    printf("\n");

    printf("Diagonal Down: ");
    for (uint32_t i = 0; i < (2 * board->n_size - 1); i++)
    {
        printf("%d ", board->diagonal_down[i]);
    }
    printf("\n\n");
}

struct chess_board *copyBoard(struct chess_board *board)
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

  return task;
}


int tasks_created = 0;



int total_solutions = 0;
static void place_next_queen_seq(const uint32_t row_boundary, struct chess_board *boardy)
{
    //printf("seq version\n");
    const uint32_t middle = boardy->column_j ? boardy->n_size : boardy->n_size >> 1;

    for (uint32_t row_i = 0; row_i < row_boundary; ++row_i)
    {
        if (square_is_free(row_i, boardy))
        {
            set_queen(row_i, boardy);
            if (boardy->column_j == boardy->n_size)
            {
                //#pragma omp atomic 
                total_solutions += 2;
                boardy->solutions += 2;
            }
            else
            {
                if (boardy->queen_positions[0] != middle)
                {
                    place_next_queen_seq(boardy->n_size, boardy);
                }
                else
                {
                    place_next_queen_seq(middle, boardy);
                }
            }
            remove_queen(row_i, boardy);
        }
    }
}


void place_next_queen_parallel(const uint32_t row_boundary, struct chess_board *boardy)
{
    const uint32_t middle = boardy->column_j ? boardy->n_size : boardy->n_size >> 1;

    for (uint32_t row_i = 0; row_i < row_boundary; ++row_i)
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
                    if(local_board->column_j < 6) {
                        if (local_board->queen_positions[0] != middle) {
                            place_next_queen_parallel(local_board->n_size, local_board);
                        } else {
                            place_next_queen_parallel(middle, local_board);
                        }
                    } else {
                        if (local_board->queen_positions[0] != middle) {
                            place_next_queen_seq(local_board->n_size, local_board);
                        } else {
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

void place_next_queen(const uint32_t row_boundary, struct chess_board *board)
{
    
    #pragma omp parallel
    #pragma omp single nowait
    {
        place_next_queen_parallel(row_boundary, board);
    }
    
}



int main(int argc, char *argv[])
{

    double temps_debut, temps_fin;
    long double temps_total_pris = 0;
    struct chess_board *board;

    static const uint32_t default_n = 8;
    const uint32_t n_queens = (argc != 1) ? (uint32_t)atoi(argv[1]) : default_n;
    initialize_board(n_queens, &board);

    // Determines the index for the middle row to take advantage of board
    // symmetry when searching for solutions
    const uint32_t row_boundary = (n_queens >> 1) + (n_queens & 1);
    omp_set_num_threads(8);

    temps_debut = clock(); // omp_get_wtime();
    // #pragma omp parallel
    place_next_queen(row_boundary, board); // Start solver algorithm

    temps_fin = clock(); // omp_get_wtime();

    temps_total_pris = (temps_fin - temps_debut);
    // print_counts(board);

    printf("Temps pris: %Lf\n", temps_total_pris);

    smash_board(board); // Free dynamically allocated memory

    printf("final %d\n", total_solutions);
    printf("tasks created %d \n", tasks_created);

    return EXIT_SUCCESS;
}