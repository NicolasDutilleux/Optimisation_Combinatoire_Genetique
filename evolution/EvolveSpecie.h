// evolution/EvolveSpecie.h - Pure C version
#ifndef EVOLVESPECIE_H
#define EVOLVESPECIE_H

#include "core\Individual.h"
#include "core\Node.h"

void EvolveSpecie(
    Individual* specie, int specie_size,
    const double** dist,
    const int** ranking,
    const Node* nodes, int num_nodes,
    int alpha,
    int total_stations,
    double mutation_rate,
    int elitism_count,
    int add_percentage,
    int remove_percentage,
    int swap_percentage,
    int inversion_percentage,
    int scramble_percentage,
    double mating_pool_fraction);

// Sanity function to test callability
void EvolveSpecie_Sanity(Individual* specie);

#endif
