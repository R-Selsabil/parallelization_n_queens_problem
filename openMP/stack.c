

#include "stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>



// Initialize a new stack
struct stack *new_stack() {
    struct stack *s = (struct stack *)malloc(sizeof(struct stack));
    s->top = NULL;
    return s;
}

// Push a new element onto the stack
void push(struct stack *s, struct chess_board *data) {
    struct node *new_node = (struct node *)malloc(sizeof(struct node));
    new_node->data = data;
    new_node->next = s->top;
    s->top = new_node;
}

// Pop the top element from the stack and return its data
struct chess_board *pop(struct stack *s) {
    if (s->top == NULL) {
        return NULL;
    } else {
        struct node *temp = s->top;
        struct chess_board *data = temp->data;
        s->top = temp->next;
        free(temp);
        return data;
    }
}

int test_stack() {
    // Create a new chess board
    struct chess_board *board = (struct chess_board *)malloc(sizeof(struct chess_board));
    board->n_size = 8;
    board->queen_positions = (uint32_t *)malloc(board->n_size * sizeof(uint32_t));
    board->column = (uint32_t *)malloc(board->n_size * sizeof(uint32_t));
    board->diagonal_up = (uint32_t *)malloc((2 * board->n_size - 1) * sizeof(uint32_t));
    board->diagonal_down = (uint32_t *)malloc((2 * board->n_size - 1) * sizeof(uint32_t));
    board->column_j = 0;
    board->placements = 0;
    board->solutions = 0;

    // Create a new stack
    struct stack *s = new_stack();

    // Push the chess board onto the stack
    push(s, board);

    // Pop the chess board from the stack and free its memory
    struct chess_board *popped_board = pop(s);
    free(popped_board->queen_positions);
    free(popped_board->column);
    free(popped_board->diagonal_up);
    free(popped_board->diagonal_down);
    free(popped_board);
    return 0;
}


