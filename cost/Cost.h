#pragma once
#include "core\Individual.h"

double Cost_station(int alpha, double distance);
double Cost_out_ring(int alpha, double distance);

double RingCostOnly(int alpha, const int* active_ring, int ring_size, const double** dist, int dist_size);

double OutRingCostOnly(int alpha, int total_stations, const int* active_ring, int ring_size, 
                       const double** dist, const int** ranking);

double Total_Cost_Individual(int alpha, const Individual* ind, int total_stations,
                             const double** dist, const int** ranking);

double* Total_Cost_Specie(int alpha, Individual* specie, int specie_size, int total_stations,
                          const double** dist, const int** ranking);

