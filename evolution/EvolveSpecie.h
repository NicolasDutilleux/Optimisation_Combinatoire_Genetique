#pragma once
#include <vector>

#include "core/Individual.h"
#include "core/Node.h"

void EvolveSpecie(
    std::vector<Individual>& specie,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking,
    const std::vector<Node>& nodes,
    int alpha,
    int total_stations,
    double mutation_rate,
    int elitism_count = 1,
    int add_percentage = 15,
    int remove_percentage = 10,
    int swap_percentage = 10,
    int inversion_percentage = 10,
    int scramble_percentage = 10,
    double mating_pool_fraction = 0.5);