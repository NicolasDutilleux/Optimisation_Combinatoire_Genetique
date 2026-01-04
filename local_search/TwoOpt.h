#pragma once
#include <vector>
#include "core/Individual.h"

// 2-opt complet (plus lent par itération, mais convergence bien meilleure)
void TwoOptImproveAlpha(
    Individual& ind,
    int alpha,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking);