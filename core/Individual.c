// core/Individual.c - Pure C implementation
#include "Individual.h"
#include <stdlib.h>
#include <string.h>

Individual* Individual_Create(int capacity)
{
    Individual* ind = (Individual*)malloc(sizeof(Individual));
    if (!ind) return NULL;
    Individual_Init(ind, capacity);
    return ind;
}

void Individual_Init(Individual* ind, int capacity)
{
    if (!ind) return;
    ind->active_ring = (int*)malloc(capacity * sizeof(int));
    if (!ind->active_ring) {
        ind->ring_size = 0;
        ind->ring_capacity = 0;
        ind->cached_cost = 1e18;
        return;
    }
    ind->ring_size = 0;
    ind->ring_capacity = capacity;
    ind->cached_cost = 1e18;
}

void Individual_Free(Individual* ind)
{
    if (!ind) return;
    if (ind->active_ring) {
        free(ind->active_ring);
        ind->active_ring = NULL;
    }
    ind->ring_size = 0;
    ind->ring_capacity = 0;
    ind->cached_cost = 1e18;
}

void Individual_Copy(Individual* dest, const Individual* src)
{
    if (!dest || !src) return;

    if (src->ring_size > dest->ring_capacity) {
        int* new_ring = (int*)realloc(dest->active_ring, src->ring_size * sizeof(int));
        if (!new_ring) return;
        dest->active_ring = new_ring;
        dest->ring_capacity = src->ring_size;
    }

    memcpy(dest->active_ring, src->active_ring, src->ring_size * sizeof(int));
    dest->ring_size = src->ring_size;
    dest->cached_cost = src->cached_cost;
}
