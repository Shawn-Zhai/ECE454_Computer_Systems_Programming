/*
 * ECE454 Lab 3 - Malloc
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
 *************************************************************************/
#define WSIZE       sizeof(void *)            /* word size (bytes) */
#define DSIZE       (2 * WSIZE)               /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)                    /* initial heap size (bytes) */
#define LIST_LIMIT  12

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(uintptr_t *)(p))
#define PUT(p,val)      (*(uintptr_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Given free block ptr bp, compute address of next and previous free blocks */
#define NEXT_FREE(bp)  (*(void **)(bp))
#define PREV_FREE(bp)  (*(void **)((char *)(bp) + WSIZE))

void *heap_listp = NULL;
void *segregated_free_lists[LIST_LIMIT];

// insert a free block into the appropriate segregated list
void insert_free_block(void *bp) {
    size_t size = GET_SIZE(HDRP(bp));
    int list = 0;

    while ((list < LIST_LIMIT - 1) && (size > 1 << (list + 4))) {
        list++;
    }

    NEXT_FREE(bp) = segregated_free_lists[list];
    PREV_FREE(bp) = NULL;
    if (segregated_free_lists[list] != NULL) {
        PREV_FREE(segregated_free_lists[list]) = bp;
    }
    segregated_free_lists[list] = bp;
}

// remove a free block from the segregated list
void remove_free_block(void *bp) {
    int list = 0;
    size_t size = GET_SIZE(HDRP(bp));

    while ((list < LIST_LIMIT - 1) && (size > 1 << (list + 4))) {
        list++;
    }

    if (NEXT_FREE(bp) != NULL) {
        PREV_FREE(NEXT_FREE(bp)) = PREV_FREE(bp);
    }
    if (PREV_FREE(bp) != NULL) {
        NEXT_FREE(PREV_FREE(bp)) = NEXT_FREE(bp);
    } else {
        segregated_free_lists[list] = NEXT_FREE(bp);
    }
}

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void) 
{
    for (int i = 0; i < LIST_LIMIT; i++) {
        segregated_free_lists[i] = NULL;
    }

    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
        return -1;

    PUT(heap_listp, 0);                         // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));   // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));   // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));       // epilogue header
    heap_listp += DSIZE;

    return 0;
}

/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 **********************************************************/
void *coalesce(void *bp) 
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {       /* Case 1 */
        insert_free_block(bp);
        return bp;
    } 
    
    else if (prev_alloc && !next_alloc) { /* Case 2 */
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        remove_free_block(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    } 
    
    else if (!prev_alloc && next_alloc) { /* Case 3 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        remove_free_block(PREV_BLKP(bp));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    } 
    
    else {            /* Case 4 */
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        remove_free_block(PREV_BLKP(bp));
        remove_free_block(NEXT_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    insert_free_block(bp);
    return bp;
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words) 
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((bp = mem_sbrk(size)) == (void *)-1) {
        return NULL;
    }

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));                // free block header
    PUT(FTRP(bp), PACK(size, 0));                // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/**********************************************************
 * find_fit
 * Traverse the segregated lists searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void *find_fit(size_t asize) 
{
    int list = 0;
    void *bp = NULL;
    while ((list < LIST_LIMIT - 1) && (asize > 1 << (list + 4))) {
        list++;
    }

    for (; list < LIST_LIMIT; list++) {
        bp = segregated_free_lists[list];
        
        while (bp != NULL) {
            
            if (asize <= GET_SIZE(HDRP(bp))) {
                return bp;
            }
            bp = NEXT_FREE(bp);
        }
    }
    return NULL;
}

/**********************************************************
 * place
 * Mark the block as allocated, and split if the remainder
 * would be at least the minimum block size
 **********************************************************/
void place(void *bp, size_t asize) 
{
    size_t bsize = GET_SIZE(HDRP(bp));

    remove_free_block(bp);

    if ((bsize - asize) >= (2 * DSIZE)) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(bsize - asize, 0));
        PUT(FTRP(bp), PACK(bsize - asize, 0));
        insert_free_block(bp);
    } else {
        PUT(HDRP(bp), PACK(bsize, 1));
        PUT(FTRP(bp), PACK(bsize, 1));
    }
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp) 
{
    if (bp == NULL) {
        return;
    }
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 **********************************************************/
void *mm_malloc(size_t size) 
{
    size_t asize;      /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char *bp;

    /* Ignore spurious requests */
    if (size == 0) 
        return NULL;

    /* Adjust block size to include overhead and alignment requirements */
    if (size <= DSIZE) 
        asize = 2 * DSIZE;
    else 
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    /* Search the segregated list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL) {
        return NULL;
    }
    place(bp, asize);
    return bp;
}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size) 
{
    /* If size == 0 then this is just free, and we return NULL. */
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL) 
        return mm_malloc(size);

    size_t old_size = GET_SIZE(HDRP(ptr));
    size_t new_size = size <= DSIZE ? 2 * DSIZE : DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    if (new_size <= old_size) {
        return ptr;
    }

    // check if coalescing with the next block is possible
    void *next_block = NEXT_BLKP(ptr);
    if (!GET_ALLOC(HDRP(next_block)) && (old_size + GET_SIZE(HDRP(next_block)) >= new_size)) {
        size_t combined_size = old_size + GET_SIZE(HDRP(next_block));
        remove_free_block(next_block);
        PUT(HDRP(ptr), PACK(combined_size, 1));
        PUT(FTRP(ptr), PACK(combined_size, 1));
        return ptr;
    }

    void *newptr = mm_malloc(size);
    if (newptr == NULL) {
        return NULL;
    }
    memcpy(newptr, ptr, old_size - DSIZE);
    mm_free(ptr);
    return newptr;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistent.
 *********************************************************/
int mm_check(void) {
    void *bp = heap_listp;
    
    for (; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if ((uintptr_t)bp % 16 != 0) {
            printf("Error: Block at %p is not aligned\n", bp);
            return 0;
        }
        if (!GET_ALLOC(HDRP(bp)) && !GET_ALLOC(HDRP(NEXT_BLKP(bp)))) {
            printf("Error: Two consecutive free blocks found at %p and %p\n", bp, NEXT_BLKP(bp));
            return 0;
        }
    }
    return 1;
}
