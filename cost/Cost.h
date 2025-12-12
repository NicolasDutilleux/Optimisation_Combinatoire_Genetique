#pragma once
#include <vector>
#include "core/Individual.h"

double Cost_station(int alpha, double distance);
double Cost_out_ring(int alpha, double distance);

// Ring cost only (depends on ids order + mask)
double RingCostOnly(int alpha,
    const std::vector<int>& perm,
    const std::vector<bool>& mask,
    const std::vector<std::vector<double>>& dist);

// Out-of-ring cost only (depends on mask only + ranking)
double OutRingCostOnly(int alpha,
    const std::vector<bool>& mask,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector);

double Total_Cost_Individual(int alpha,
    const std::vector<int>& perm,
    const std::vector<bool>& mask,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector);

std::vector<double> Total_Cost_Specie(int alpha,
    std::vector<Individual>& specie,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking);

// For 2-opt (active tour only)
double RingCostActiveTour(int alpha,
    const std::vector<int>& active_ids,
    const std::vector<std::vector<double>>& dist);

