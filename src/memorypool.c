#include <string.h>
#include "memorypool.h"
#include "zen_internal.h"

static void memorypool_resize(struct memorypool *);

struct memorypool *memorypool_new(size_t blocks_nr, size_t blocksize) {
    struct memorypool *pool = zen_malloc(sizeof(*pool));
    if (!pool)
        return NULL;
    blocksize = blocksize >= sizeof(intptr_t) ? blocksize : sizeof(intptr_t);
    pool->memory = zen_calloc(blocks_nr, blocksize);
    pool->free = pool->memory;
    pool->blocks_nr = blocks_nr;
    pool->blocksize = blocksize;
    if (!pool->free) {
        zen_free(pool);
        return NULL;
    }
    /*
     * We pre-assign the position of each free block in the free pointer, this
     * way we know every block position before allocating new memory, we'll
     * call it the header of each block, where we store the offset in memory
     * to reach the next free slot:
     *
     *       ____________
     *     _| 0x1ad45f02 | (0+blocksize)
     *    | |------------|
     *    | |     .      |
     *    | |     .      |
     *    |_|------------|
     *     _| 0x2ff43da1 | (1+blocksize)
     *    | |------------|
     *    | |     .      |
     *    | |     .      |
     *    |_|------------|
     *      | 0x98fff34a | (n+blocksize)
     *      |------------|
     *      |     .      |
     *
     * Just before assigning a free block of memory, we update the free pointer,
     * pointing it to the memory address previously stored as r-value in it.
     * This way everytime we allocate a new block we can refresh the next free
     * block in the list.
     */
    intptr_t *ptr = pool->free;
    for (size_t i = 1; i != blocks_nr; ++i) {
        *ptr = (intptr_t) i;
        ptr = (intptr_t *)((char *) ptr + blocksize);
    }
    pool->block_used = 0;
    return pool;
}

void memorypool_destroy(struct memorypool *pool) {
    zen_free(pool->memory);
    zen_free(pool);
}

void *memorypool_alloc(struct memorypool *pool) {
    if (pool->block_used == pool->blocks_nr - 2)
        memorypool_resize(pool);
    void *ptr = pool->free;
    /*
     * After pointing the return pointer to the next free block, we need to
     * update the next free block address on the free pointer. The address is
     * already stored in the "header" of the block.
     */
    pool->free = (intptr_t *)((char *) pool->memory +
                              (*((intptr_t *) pool->free)) * pool->blocksize);
    pool->block_used++;
    return ptr;
}

void memorypool_free(struct memorypool *pool, void *ptr) {
    /*
     * Here we just need to point the header of the pointer to the next free
     * location and udpate the current free location by pointing it to the
     * free'd pointer
     */
    *((intptr_t *) ptr) = *((intptr_t *) pool->free);
    if (pool->block_used == pool->blocks_nr - 2)
        memorypool_resize(pool);
    pool->free = ptr;
    pool->block_used--;
}

void *memorypool_advance_pointer(struct memorypool *pool, size_t i) {
    void *ptr = (intptr_t *) ((char *) pool->memory + i * pool->blocksize);
    return ptr;
}

static void memorypool_resize(struct memorypool *pool) {
    pool->blocks_nr *= 2;
    size_t newsize = pool->blocks_nr * pool->blocksize;
    /* We extract next memory block offset position */
    intptr_t offset = *((intptr_t *) pool->free);
    pool->memory = zen_realloc(pool->memory, newsize);
    pool->free = (void *)((char *) pool->memory + ((offset-1) * pool->blocksize));
    /*
     * Apply the same logic of the init, but starting from the updated offset,
     * the ald size of the pool
     */
    intptr_t *ptr = (intptr_t *) pool->free;
    for (size_t i = offset; i <= pool->blocks_nr; ++i) {
        *ptr = (intptr_t) i;
        ptr = (intptr_t *)((char *) ptr + pool->blocksize);
    }
}
