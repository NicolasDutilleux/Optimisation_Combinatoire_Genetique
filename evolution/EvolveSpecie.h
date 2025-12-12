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
    double mutation_rate,
    int max_station_id,
    int elitism_count = 1,
    int DEL_PCT_RDM = 10,
    int DEL_PCT_CENT = 10,
    int SWAP_PCT = 10,
    int INV_PCT = 10,
    int SCR_PCT = 10,
    int INSERTSWAP_PCT = 10,
    double max_mating_pool_size = 0.5,
    bool time_bool = true
);
