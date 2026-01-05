// local_search/TwoOpt.h - Pure C version
#ifndef TWOOPT_H
#define TWOOPT_H

#include "core\Individual.h"

// 2-opt local search on active_ring (alpha-weighted)
void TwoOptImproveAlpha(
    Individual* ind,
    int alpha,
    const double** dist,
    const int** ranking);

#endif

