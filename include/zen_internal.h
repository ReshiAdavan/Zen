#ifndef ZEN_INTERNAL
#define ZEN_INTERNAL

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#define RANDOM(A,B) A + rand() / (RAND_MAX / (B - A))

/* Load-balancing mode */
#define ZEN_TCP_MODE  0
#define ZEN_HTTP_MODE 1

#define ZEN_SUCCESS  0
#define ZEN_FAILURE -1

/* Load-balancing algorithms */
#define ROUND_ROBIN          0
#define HASH_BALANCING       1
#define RANDOM_BALANCING     2
#define LEASTCONN            3
#define LEASTTRAFFIC         4
#define WEIGHTED_ROUND_ROBIN 5

static inline void *zen_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr && size != 0) {
        fprintf(stderr, "[%s:%ul] Out of memory (%lu bytes)\n", __FILE__, __LINE__, size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static inline void *zen_calloc(size_t n, size_t size) {
    void *ptr = calloc(n, size);
    if (!ptr && size != 0 && n != 0) {
        fprintf(stderr, "[%s:%ul] Out of memory (%lu bytes)\n",
                __FILE__, __LINE__, size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}

static inline void *zen_realloc(void *ptr, size_t size) {
    assert(ptr && size > 0);
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr && size != 0) {
        fprintf(stderr, "[%s:%ul] Out of memory (%lu bytes)\n",
                __FILE__, __LINE__, size);
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

static inline void zen_free(void *ptr) {
    free(ptr);
}

/* D. J. Bernstein hash function */
static inline size_t djb_hash(const char *str) {
    size_t hash = 5381;
    while (*str)
        hash = 33 * hash ^ (unsigned char) *str++;
    return hash;
}

#endif
