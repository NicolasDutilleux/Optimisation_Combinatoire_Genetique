#pragma once
#include <vector>
#include "core/Node.h"

// Distance matrix
std::vector<std::vector<double>>
Compute_Distances_2DVector(const std::vector<Node>& node_vector);

// If you keep this (you can later remove it), it belongs here
void Bubble_Sort(std::vector<int>& vector_int_to_sort,
    std::vector<double>& vector_double_to_sort);

// Ranking by distance (for each city, list of IDs sorted by increasing distance)
std::vector<std::vector<int>>
Distance_Ranking_2DVector(std::vector<std::vector<double>> distance_vector);

// Nearest active station using precomputed ranking
int Find_Nearest_Station(int city_id,
    const std::vector<bool>& mask,
    const std::vector<std::vector<int>>& ranking_vector);

// Assignment list: (inactive_city, nearest_active_station) pairs in a flat vector
std::vector<int>
Assign_Stations(int number_of_stations,
    const std::vector<bool>& mask,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector);
