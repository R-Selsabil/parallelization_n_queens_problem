#include "n_queens_counter_lib.h"
#include <omp.h>

int tasks_created = 0;

uint32_t nqueens = 8; 

int nthreads = 8;

int partial_solutions = 0;

void place_next_queen_sequential(const uint32_t row_boundary, CHESS_BOARD *board);
void place_next_queen_parallel(const uint32_t row_boundary, CHESS_BOARD *board);
void place_queens(const uint32_t row_boundary, CHESS_BOARD *board);




#pragma omp threadprivate(partial_solutions)

/** Lancer le processus de placement parallèle des reines sur l'échiquier. **/
void place_queens(const uint32_t row_boundary, CHESS_BOARD *board)

{
    //crée une région parallèle avec nqueens threads
    #pragma omp parallel num_threads(nthreads) 
    {
        //Initialiser le nombre de solutions partielles et de placements partiels pour chaque thread
        partial_solutions = 0;
        
        //Assurer que la ligne suivante soit appelée par un seul thread et que les autres threads ne l'attendent pas
        #pragma omp single 
        {
            //Fonction parallélisée permettant de placer la reine suivante sur l'échiquier board.
            place_next_queen_parallel(row_boundary, board);
        }

        //Additionner le nombre de solutions et de placements trouvés par chaque thread
        #pragma omp atomic
        total_solutions += partial_solutions;
    }

    
}


//la partie parallèle de l'algorithme, cette fonction génère des tâches à exécuter pour les threads 
void place_next_queen_parallel(const uint32_t row_boundary, CHESS_BOARD *board) {
    const uint32_t middle = board->column_j ? board->n_size : board->n_size >> 1;
    
    //échiquier local 
    CHESS_BOARD *local_board;
    for (uint32_t row_i = 0; row_i < row_boundary; ++row_i) {  
        //si la position est valide 
        if (square_is_free(row_i, board)) 
        {

            //créer une tâche pour placer une reine
            #pragma omp task firstprivate(row_i) mergeable 
            {       
                //copier l'échequier actuelle pour donner à la tâche une zone mémoire
                local_board = copyBoard(board);

                //placer la reine dans l'échiquier local
                set_queen(row_i, local_board);

                //définir la limite pour la recherche dans la colonne suivante
                uint32_t limit = local_board->n_size;
                if (local_board->queen_positions[0] == middle) {
                    limit = middle;
                }

                //définir où arrêter la création de tâches 
                //parce que les tâches deviennent plus petites au fur et à mesure que l'on progresse dans l'échiquier
                //
                if(local_board->column_j < nqueens / 4 + 1) { 
                    //on crée des tâches tant que l'on est dans le premier quart du tableau + 1
                    place_next_queen_parallel(limit, local_board);
                } else {
                    //sinon, nous laissons chaque thread continuer son travail
                    place_next_queen_sequential(limit, local_board);
                }

            //Déléguer l'exécution de la tâche en cours aux threads disponibles.
            #pragma omp taskyield
            
            }
            //libérer la zone mémoire locale
            free(local_board); 
        }
    }
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
                partial_solutions += 2;
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

    if (argc != 3) {
        printf("Invalid number of arguments. Usage: program_name <nqueens> <nthreads>\n");
        return 1;
    }

    nqueens = (uint32_t)atoi(argv[1]);
    nthreads = (uint32_t)atoi(argv[2]);

    const uint32_t row_boundary = (nqueens >> 1) + (nqueens & 1);


    initialize_board(nqueens, &board);

    place_queens(row_boundary, board); 

    smash_board(board);


    return EXIT_SUCCESS;
}