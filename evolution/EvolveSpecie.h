// evolution/EvolveSpecie.h
// Header for genetic evolution operators
//
// Usage:
//   EvolveSpecie(specie, size, dist, ranking, nodes, num_nodes,
//                alpha, total_stations, mutation_rate, elitism,
//                add%, remove%, swap%, inv%, scramble%,
//                mating_pool_fraction, enable_logs, enable_timers);

#ifndef EVOLVESPECIE_H
#define EVOLVESPECIE_H

#include "core\Individual.h"
#include "core\Node.h"

// Evolve a species for one generation
// 
// Parameters:
//   specie              - Array of individuals (modified in place)
//   specie_size         - Number of individuals
//   dist                - Distance matrix [i][j] = distance between i and j
//   ranking             - Sorted neighbors [i][k] = k-th nearest to i
//   nodes               - Node coordinates (unused, kept for compatibility)
//   num_nodes           - Number of nodes (unused)
//   alpha               - Cost weighting parameter
//   total_stations      - Total number of stations (1-indexed)
//   mutation_rate       - Probability of mutation [0.0, 1.0]
//   elitism_count       - Number of best individuals to preserve
//   add_percentage      - Weight for add-node mutation
//   remove_percentage   - Weight for remove-node mutation
//   swap_percentage     - Weight for swap mutation
//   inversion_percentage - Weight for inversion mutation
//   scramble_percentage - Weight for scramble mutation
//   mating_pool_fraction - Fraction of population in mating pool
//   enable_logs         - Print detailed logs (0=off, 1=on)
//   enable_timers       - Print timing info (0=off, 1=on)
//

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
    double mating_pool_fraction,
    int enable_logs,
    int enable_timers
);

// Test function to verify linkage
void EvolveSpecie_Sanity(Individual* specie);

#endif // EVOLVESPECIE_H
