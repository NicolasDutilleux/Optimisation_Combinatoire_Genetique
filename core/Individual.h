#pragma once

typedef struct {
    int* active_ring;
    int ring_size;
    int ring_capacity;
    double cached_cost;
} Individual;

// Allocate a new Individual on heap
Individual* Individual_Create(int capacity);
// Initialize an existing Individual struct (allocates internal buffer)
void Individual_Init(Individual* ind, int capacity);
// Free internal buffers but DO NOT free the struct itself
void Individual_Free(Individual* ind);
// Copy contents from src to dest (dest must have sufficient capacity or will be reallocated)
void Individual_Copy(Individual* dest, const Individual* src);