// genetic/Crossover.h - Pure C version
#ifndef CROSSOVER_H
#define CROSSOVER_H

// Slice crossover: take a slice from parent A, fill rest from parent B
// Returns child ring (caller must free)
int* Slice_Crossover(
    const int* parentA, int sizeA,
    const int* parentB, int sizeB,
    int* child_size, int max_capacity);

#endif
