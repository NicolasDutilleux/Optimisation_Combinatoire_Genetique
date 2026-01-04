#pragma once
#include <vector>
#include "core/Node.h"

std::vector<std::vector<double>>
Compute_Distances_2DVector(const std::vector<Node>& node_vector);

void Bubble_Sort(std::vector<int>& vector_int_to_sort,
    std::vector<double>& vector_double_to_sort);

std::vector<std::vector<int>>
Distance_Ranking_2DVector(std::vector<std::vector<double>> distance_vector);

int Find_Nearest_Station(int city_id,
    const std::vector<int>& active_ring,
    const std::vector<std::vector<int>>& ranking_vector);

std::vector<int>
Assign_Stations(int total_stations,
    const std::vector<int>& active_ring,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector);
