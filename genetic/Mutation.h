// genetic/Mutation.h - Pure C version
#ifndef MUTATION_H
#define MUTATION_H

#include "core\Individual.h"
#include "core\Node.h"

// Add a random inactive station at the best insertion position
void Mutation_Add_Node(
    Individual* ind,
    int total_stations,
    const double** dist);

// Remove a random station from the ring (keeping at least 3)
void Mutation_Remove_Node(Individual* ind, int min_ring_size);

// Permutation mutations
void Mutation_Swap_Simple(Individual* ind);
void Mutation_Inversion(Individual* ind);
void Mutation_Scramble(Individual* ind);

// All mutations bundled together
void Mutations(
    Individual* out_individual,
    int add_percentage,
    int remove_percentage,
    int swap_percentage,
    int inversion_percentage,
    int scramble_percentage,
    const Individual* individual,
    int total_stations,
    const double** dist);

#endif

