#pragma once

#include <vector>
#include "core/Individual.h"

// Improves an individual's tour using 2-opt (alpha-weighted)
void TwoOptImproveAlpha(
    Individual& ind,
    int alpha,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking
);

// Inserts a station at the best position in a tour (used by mask local search)
void InsertStationBestPos(
    std::vector<int>& perm,
    int station_id,
    const std::vector<std::vector<double>>& dist
);

