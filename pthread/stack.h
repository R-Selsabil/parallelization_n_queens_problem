
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifndef __biblioHPC__
#define __biblioHPC__

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

struct node {
    struct chess_board *data;
    struct node *next;
};

struct stack {
    struct node *top;
};

struct stack *new_stack();
void push(struct stack *s, struct chess_board *data);
struct chess_board *pop(struct stack *s);
int test_stack();

#endif
