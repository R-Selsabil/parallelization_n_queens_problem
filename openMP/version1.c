
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
struct chess_board {
  uint32_t n_size;            // Number of queens on the NxN chess board
  uint32_t *queen_positions;  // Store queen positions on the board
  uint32_t *column;           // Store available column moves/attacks
  uint32_t *diagonal_up;      // Store available diagonal moves/attacks
  uint32_t *diagonal_down;
  uint32_t column_j;          // Stores column to place the next queen in
  uint64_t placements;        // Tracks total number queen placements
  uint64_t solutions;         // Tracks number of solutions
};
static struct chess_board *board;

// Handles dynamic memory allocation of the arrays and sets initial values
static void initialize_board(const uint32_t n_queens) {
  if (n_queens < 1) {
    fprintf(stderr, "The number of queens must be greater than 0.\n");
    exit(EXIT_SUCCESS);
  }

  // Dynamically allocate memory for chessboard struct
  board = malloc(sizeof(struct chess_board));
  if (board == NULL) {
    fprintf(stderr, "Memory allocation failed for chess board.\n");
    exit(EXIT_FAILURE);
  }

  // Dynamically allocate memory for chessboard arrays that track positions
  const uint32_t diagonal_size = 2 * n_queens - 1;
  const uint32_t total_size = 2 * (n_queens + diagonal_size);
  board->queen_positions = malloc(sizeof(uint32_t) * total_size);
  if (board->queen_positions == NULL) {
    fprintf(stderr, "Memory allocation failed for the chess board arrays.\n");
    free(board);
    exit(EXIT_FAILURE);
  }
  board->column = &board->queen_positions[n_queens];
  board->diagonal_up = &board->column[n_queens];
  board->diagonal_down = &board->diagonal_up[diagonal_size];

  // Initialize the chess board parameters
  board->n_size = n_queens;

  for (uint32_t i = 0; i < n_queens; ++i) {
    board->queen_positions[i] = 0;
  }
  for (uint32_t i = n_queens; i < total_size; ++i) {
    // Initializes values for column, diagonal_up, and diagonal_down
    board->queen_positions[i] = 1;
  }
  board->column_j = 0;
  board->placements = 0;
  board->solutions = 0;


}

// Frees the dynamically allocated memory for the chess board structure
static void smash_board(struct chess_board *board) {
  free(board->queen_positions);
  free(board);
}

// Check if a queen can be placed in at row 'i' of the current column
static uint32_t square_is_free(const uint32_t row_i, struct chess_board *board) {
  return board->column[row_i] &
         board->diagonal_up[(board->n_size - 1) + (board->column_j - row_i)] &
         board->diagonal_down[board->column_j + row_i];
}

// Place a queen on the chess board at row 'i' of the current column
static void set_queen(const uint32_t row_i, struct chess_board *board) {
  board->queen_positions[board->column_j] = row_i;
  board->column[row_i] = 0;
  board->diagonal_up[(board->n_size - 1) + (board->column_j - row_i)] = 0;
  board->diagonal_down[board->column_j + row_i] = 0;
  ++board->column_j;
  ++board->placements;
}

// Removes a queen from the NxN chess board in the given column to backtrack
static void remove_queen(const uint32_t row_i, struct chess_board *board) {
  --board->column_j;
  board->diagonal_down[board->column_j + row_i] = 1;
  board->diagonal_up[(board->n_size - 1) + (board->column_j - row_i)] = 1;
  board->column[row_i] = 1;
}

// Prints the number of queen placements and solutions for the NxN chess board
static void print_counts(struct chess_board *board) {
  // The next line fixes double-counting when solving the 1-queen problem
  const uint64_t size = board->n_size;
  const uint64_t sol = board->solutions;
  const uint64_t solution_count = size == 1 ? 1 : sol;
  printf("size  %lu \n", board->n_size);
  printf("solutions  %lu \n", solution_count);
  printf("placements  %lu \n", board->placements);
  printf("\n");
  
}

void printQueenPositions(struct chess_board *board) {
  printf("positions ");
  for (uint32_t i = 0; i < board->n_size; i++) {
      printf("%d ", board->queen_positions[i]);
    }
  printf("\n");
  
  printf("Column: ");
  for (uint32_t i = 0; i < board->n_size; i++) {
    printf("%d ", board->column[i]);
  }
  printf("\n");

  printf("Diagonal Up: ");
  for (uint32_t i = 0; i < (2 * board->n_size - 1); i++) {
    printf("%d ", board->diagonal_up[i]);
  }
  printf("\n");

  printf("Diagonal Down: ");
  for (uint32_t i = 0; i < (2 * board->n_size - 1); i++) {
    printf("%d ", board->diagonal_down[i]);
  }
  printf("\n");
}



uint64_t total_solutions = 0;

static void place_next_queen_parallel(const uint32_t row_boundary, int niv, int solutions, struct chess_board *boardy) {
  const uint32_t middle = boardy->column_j ? boardy->n_size : boardy->n_size >> 1;

  if(niv == 0){

      #pragma omp parallel for schedule(static) num_threads(8) reduction(+:total_solutions)
      for (uint32_t row_i = 0; row_i < row_boundary; ++row_i) {
        
        //create a private copy of local_board
        struct chess_board *local_board = (struct chess_board*) malloc(sizeof(struct chess_board));
        memcpy (local_board, boardy, sizeof(struct chess_board)); 
        #pragma omp critical  
        if (square_is_free(row_i, local_board)) {
          set_queen(row_i, local_board);
          if (local_board->column_j == local_board->n_size) {
            solutions += 2;
            local_board->solutions += 2;
          } else 
          { 
                if (local_board->queen_positions[0] != middle) {
                  niv++;
                  place_next_queen_parallel(local_board->n_size, niv, solutions, local_board);
                } 
                else {
                  niv++;
                  place_next_queen_parallel(middle, niv, solutions, local_board);
                }
            
          }
          remove_queen(row_i, local_board);
        }
        
        #pragma omp critical 
        {

            printf("thread %d iteration %lu solutions %lu \n\n", omp_get_thread_num(), row_i, local_board->solutions);
            printQueenPositions(local_board);
            printf("\n");
        }
       

        total_solutions += local_board->solutions;
        free(local_board);

        
  }
    
  
  } else {

      for (uint32_t row_i = 0; row_i < row_boundary; ++row_i) {
        
        if (square_is_free(row_i, boardy)) {
          set_queen(row_i, boardy);
          if (boardy->column_j == boardy->n_size) {
            
            boardy->solutions += 2;
          } else 
          {
                if (boardy->queen_positions[0] != middle) {
                  niv++;
                  place_next_queen_parallel(boardy->n_size, niv, solutions, boardy);
                } 
                else {
                  niv++;
                  place_next_queen_parallel(middle, niv, solutions, boardy);
                }
            
          }
          remove_queen(row_i, boardy);
        }
   
  }


  }
  

}


int main(int argc, char *argv[]) {

  double temps_debut, temps_fin;
	long double temps_total_pris = 0;

  static const uint32_t default_n = 8;
  const uint32_t n_queens = (argc != 1) ? (uint32_t)atoi(argv[1]) : default_n;
  initialize_board(n_queens);



 
  // Determines the index for the middle row to take advantage of board
  // symmetry when searching for solutions
  const uint32_t row_boundary = (n_queens >> 1) + (n_queens & 1);

  temps_debut = clock();//omp_get_wtime();
  //#pragma omp parallel 
  place_next_queen_parallel(row_boundary, 0, -1, board);  // Start solver algorithm
  
  temps_fin = clock();//omp_get_wtime();
  temps_total_pris = (temps_fin- temps_debut);
  print_counts(board);

  printf("Temps pris: %Lf\n", temps_total_pris);

  
  smash_board(board);  // Free dynamically allocated memory


    printf("final %lu\n", total_solutions);

  return EXIT_SUCCESS;
}