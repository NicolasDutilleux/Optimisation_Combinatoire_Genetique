#pragma once
#include <vector>
#include "core/Individual.h"

double Cost_station(int alpha, double distance);
double Cost_out_ring(int alpha, double distance);

double RingCostOnly(int alpha,
    const std::vector<int>& active_ring,
    const std::vector<std::vector<double>>& dist);

double OutRingCostOnly(int alpha,
    int total_stations,
    const std::vector<int>& active_ring,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector);

double Total_Cost_Individual(int alpha,
    const Individual& ind,
    int total_stations,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector);

std::vector<double> Total_Cost_Specie(int alpha,
    std::vector<Individual>& specie,
    int total_stations,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking);

double RingCostActiveTour(int alpha,
    const std::vector<int>& active_ring,
    const std::vector<std::vector<double>>& dist);

