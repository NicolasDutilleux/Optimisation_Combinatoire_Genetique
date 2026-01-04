#pragma once
#include <vector>
#include "core/Individual.h"

// 2-opt local search on active_ring (alpha-weighted)
void TwoOptImproveAlpha(
    Individual& ind,
    int alpha,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking);

