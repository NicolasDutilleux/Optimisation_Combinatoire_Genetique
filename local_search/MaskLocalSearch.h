#pragma once

#include <vector>

#include "core/Individual.h"

/*
 * Perform local search on the mask of an individual.
 * This may:
 *  - remove active stations
 *  - insert inactive stations
 *  - re-optimize the tour locally
 */
void ImproveMaskLocal(
    Individual& ind,
    int alpha,
    int max_station_id,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking
);



