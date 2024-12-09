/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#define _GNU_SOURCE
#include "life.h"
#include "util.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>

#define NUM_THREADS 4
#define SWAP_POINTERS( ptr1, ptr2 )  do { \
    char* temp = ptr1; \
    ptr1 = ptr2; \
    ptr2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__i) * LDA + (__j)])

/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/

// Thread arguments
typedef struct {
    char* outboard;
    char* inboard;
    int nrows;
    int ncols;
    int LDA;
    int gens_max;
    int thread_id;
} thread_args_t;

pthread_barrier_t barrier;

// calculate next generation for a portion of the board
void* compute_chunk(void* args) {
    thread_args_t* targs = (thread_args_t*)args;

    char* outboard = targs->outboard;
    char* inboard = targs->inboard;
    const int nrows = targs->nrows;
    const int ncols = targs->ncols;
    const int LDA = targs->LDA;
    const int gens_max = targs->gens_max;
    const int thread_id = targs->thread_id;

    int rows_per_thread = nrows / NUM_THREADS;
    int start_row = thread_id * rows_per_thread;
    int end_row = (thread_id == NUM_THREADS - 1) ? nrows : start_row + rows_per_thread;

	cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_id % NUM_THREADS, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    for (int curgen = 0; curgen < gens_max; curgen++) {

        // process inner cells (excluding boundary rows and columns)
        for (int i = start_row + 1; i < end_row - 1; i++) {
            for (int j = 1; j < ncols - 1; j++) {
                
                int neighbor_count =
                    BOARD(inboard, i - 1, j - 1) +
                    BOARD(inboard, i - 1, j) +
                    BOARD(inboard, i - 1, j + 1) +
                    BOARD(inboard, i, j - 1) +
                    BOARD(inboard, i, j + 1) +
                    BOARD(inboard, i + 1, j - 1) +
                    BOARD(inboard, i + 1, j) +
                    BOARD(inboard, i + 1, j + 1);

                BOARD(outboard, i, j) = (neighbor_count == 3) || (BOARD(inboard, i, j) && neighbor_count == 2);
            }
        }

        // handle boundary rows and columns with wrapping
        // top and bottom rows
        int boundary_rows[] = { start_row, end_row - 1 };
        for (int b = 0; b < 2; b++) {
            int i = boundary_rows[b];
            int inorth = (i == 0) ? nrows - 1 : i - 1;
            int isouth = (i == nrows - 1) ? 0 : i + 1;

            for (int j = 0; j < ncols; j++) {
                int jwest = (j == 0) ? ncols - 1 : j - 1;
                int jeast = (j == ncols - 1) ? 0 : j + 1;

                int neighbor_count =
                    BOARD(inboard, inorth, jwest) +
                    BOARD(inboard, inorth, j) +
                    BOARD(inboard, inorth, jeast) +
                    BOARD(inboard, i, jwest) +
                    BOARD(inboard, i, jeast) +
                    BOARD(inboard, isouth, jwest) +
                    BOARD(inboard, isouth, j) +
                    BOARD(inboard, isouth, jeast);

                BOARD(outboard, i, j) = (neighbor_count == 3) || (BOARD(inboard, i, j) && neighbor_count == 2);
            }
        }

        // leftmost and rightmost columns (excluding corners, already handled)
        for (int i = start_row + 1; i < end_row - 1; i++) {
            int j_array[] = { 0, ncols - 1 };
            for (int b = 0; b < 2; b++) {
                int j = j_array[b];
                int jwest = (j == 0) ? ncols - 1 : j - 1;
                int jeast = (j == ncols - 1) ? 0 : j + 1;
                int inorth = i - 1;
                int isouth = i + 1;

                int neighbor_count =
                    BOARD(inboard, inorth, jwest) +
                    BOARD(inboard, inorth, j) +
                    BOARD(inboard, inorth, jeast) +
                    BOARD(inboard, i, jwest) +
                    BOARD(inboard, i, jeast) +
                    BOARD(inboard, isouth, jwest) +
                    BOARD(inboard, isouth, j) +
                    BOARD(inboard, isouth, jeast);

                BOARD(outboard, i, j) = (neighbor_count == 3) || (BOARD(inboard, i, j) && neighbor_count == 2);
            }
        }

        pthread_barrier_wait(&barrier);
        SWAP_POINTERS(outboard, inboard);
    }

    return NULL;
}
/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
	pthread_t threads[NUM_THREADS];
    thread_args_t targs[NUM_THREADS];

    const int LDA = ncols;

    pthread_barrier_init(&barrier, NULL, NUM_THREADS);

    for (int t = 0; t < NUM_THREADS; t++) {
        targs[t].outboard = outboard;
        targs[t].inboard = inboard;
        targs[t].nrows = nrows;
        targs[t].ncols = ncols;
        targs[t].LDA = LDA;
        targs[t].gens_max = gens_max;
        targs[t].thread_id = t;

        pthread_create(&threads[t], NULL, compute_chunk, &targs[t]);
    }

    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    pthread_barrier_destroy(&barrier);

    return (gens_max % 2 == 0) ? inboard : outboard;
}