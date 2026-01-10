#ifndef POPULATIONINIT_H
#define POPULATIONINIT_H

#include "core\Node.h"
#include "core\Individual.h"

// Allocates 2D array of individuals (species x individuals)
// Returns array of pointers to species arrays
Individual** Random_Generation(
    const Node* node_vector, int num_nodes,
    int species_number,
    int individual_number,
    int* out_species_count);

// Apply 2-Opt to all individuals in the population (like reference code)
// This should be called right after Random_Generation for better initial quality
void Apply_TwoOpt_To_Population(Individual** species, int num_species, int pop_size,
                                 int alpha, const double** dist, const int** ranking);

// Free population
void Free_Population(Individual** species, int num_species, int ind_per_species);

#endif