#pragma once

#include <vector>
#include "core/Individual.h"

// Hill-climb on active_ring: add/remove nodes if it improves total cost
void ImproveMaskLocal(
    Individual& ind,
    int alpha,
    int total_stations,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking);



