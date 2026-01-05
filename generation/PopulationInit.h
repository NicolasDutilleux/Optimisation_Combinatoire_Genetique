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

// Free population
void Free_Population(Individual** species, int num_species, int ind_per_species);

#endif