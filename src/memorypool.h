#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

/*
 * Simple memory object-pool, the purpose is to allow for fixed size objects to
 * be pre-allocated and re-use of memory blocks, so no size have to be
 * specified like in a normal malloc but only alloc and free of a pointer is
 * possible.
 */
struct memorypool {
    void *memory;
    void *free;
    int block_used;
    size_t blocks_nr;
    size_t blocksize;
};

struct memorypool *memorypool_new(size_t, size_t);
void memorypool_destroy(struct memorypool *);
void *memorypool_alloc(struct memorypool *);
void memorypool_free(struct memorypool *, void *);
void *memorypool_advance_pointer(struct memorypool *, size_t);

#endif
