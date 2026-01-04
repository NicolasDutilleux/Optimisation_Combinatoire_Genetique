#pragma once
#include <vector>
#include "core/Node.h"

std::vector<std::vector<double>>
Compute_Distances_2DVector(const std::vector<Node>& node_vector);

// Fast distance lookup for precomputed matrix
double Get_Distance(int i, int j);

std::vector<std::vector<int>>
Distance_Ranking_2DVector(std::vector<std::vector<double>> distance_vector);

std::vector<int>
Assign_Stations(int total_stations,
    const std::vector<int>& active_ring,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector);
