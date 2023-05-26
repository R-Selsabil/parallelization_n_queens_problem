#include "n_queens_counter_lib.h"
#include <omp.h>

int tasks_created = 0;

int nqueens = 8; 

int nthreads = 8;


void place_next_queen_sequential(const uint32_t row_boundary, CHESS_BOARD *board);
void place_next_queen_parallel(const uint32_t row_boundary, CHESS_BOARD *board);
void place_queens(const uint32_t row_boundary, CHESS_BOARD *board);


/** Lancer le processus de placement parallèle des reines sur l'échiquier. **/
void place_queens(const uint32_t row_boundary, CHESS_BOARD *board)
{
    //crée une région parallèle avec nqueens threads
    #pragma omp parallel num_threads(nthreads)
    //Assurer que la ligne suivante soit appelée par un seul thread et que les autres threads ne l'attendent pas. 
    #pragma omp single nowait
    {
        //Fonction parallélisée permettant de placer la reine suivante sur l'échiquier board.
        place_next_queen_parallel(row_boundary, board);
    }
    
}



void place_next_queen_parallel(const uint32_t row_boundary, CHESS_BOARD *board) {
    const uint32_t middle = board->column_j ? board->n_size : board->n_size >> 1;

    for (uint32_t row_i = 0; row_i < row_boundary; ++row_i) {   
        if (square_is_free(row_i, board)) {
            #pragma omp task firstprivate(board, row_boundary) untied
            {       
                printf("%d\n", omp_get_thread_num());
                struct chess_board *local_board = copyBoard(board);

                set_queen(row_i, local_board);

                uint32_t limit = local_board->n_size;
                if (local_board->queen_positions[0] == middle) {
                    limit = middle;
                }


                if(local_board->column_j < local_board->n_size / 4 + 1) {
                    place_next_queen_parallel(limit, local_board);
                } else {
                    place_next_queen_sequential(limit, local_board);
                }
            }

        }

    }
    #pragma omp taskwait
}



void place_next_queen_sequential(const uint32_t row_boundary, CHESS_BOARD *board)
{
    const uint32_t middle = board->column_j ? board->n_size : board->n_size >> 1;

    for (uint32_t row_i = 0; row_i < row_boundary; ++row_i)
    {
        if (square_is_free(row_i, board))
        {
            set_queen(row_i, board);


            if (board->column_j == board->n_size){
                #pragma omp atomic  
                total_solutions += 2;
            } else {
                uint32_t limit = board->n_size;
                if (board->queen_positions[0] == middle) {
                    limit = middle;
                }

                place_next_queen_sequential(limit, board);
            }


            remove_queen(row_i, board);
        }
    }
}



int main(int argc, char *argv[])
{

    CHESS_BOARD *board;
    double start_time, end_time;
    double temps_parallel_pris, temps_sequentiel_pris = 0;

    nqueens = (argc != 1) ? (uint32_t)atoi(argv[1]) : nqueens;

    const uint32_t row_boundary = (nqueens >> 1) + (nqueens & 1);

    initialize_board(nqueens, &board);
    start_time = omp_get_wtime();
    place_next_queen_sequential(row_boundary, board); 
    end_time = omp_get_wtime();
    smash_board(board);

    temps_sequentiel_pris = (end_time - start_time);
    printf("Temps pris: %f\n", temps_sequentiel_pris);
    printf("final %d\n", total_solutions);

    
    total_solutions = 0;
    initialize_board(nqueens, &board);
    start_time = omp_get_wtime();
    place_queens(row_boundary, board); 
    end_time = omp_get_wtime();
    smash_board(board);

    temps_parallel_pris = (end_time - start_time);


    printf("Temps pris: %f\n", temps_parallel_pris);
    printf("final %d\n", total_solutions);

    
    

    printf("\nAcceleration : %f\n", temps_sequentiel_pris/temps_parallel_pris);



    return EXIT_SUCCESS;
}