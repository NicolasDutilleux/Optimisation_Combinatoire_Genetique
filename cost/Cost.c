// cost/Cost.c - Pure C implementation with safety checks
#include "Cost.h"
#include <stdlib.h>
#include <stdio.h>

double Cost_station(int alpha, double distance)
{
    return alpha * distance;
}

double Cost_out_ring(int alpha, double distance)
{
    return (10.0 - alpha) * distance;
}

double RingCostOnly(int alpha, const int* active_ring, int ring_size, 
                    const double** dist, int dist_size)
{
    if (ring_size <= 1 || !dist || !active_ring) return 0.0;

    double cost = 0.0;
    for (int i = 0; i < ring_size; ++i) {
        int id_a = active_ring[i];
        int id_b = active_ring[(i + 1) % ring_size];
        
        if (id_a <= 0 || id_a > dist_size || id_b <= 0 || id_b > dist_size) {
            fprintf(stderr, "[COST] Warning: invalid ring ids %d,%d (dist_size=%d)\n", id_a, id_b, dist_size);
            continue;
        }
            
        cost += Cost_station(alpha, dist[id_a - 1][id_b - 1]);
    }
    return cost;
}

double OutRingCostOnly(int alpha, int total_stations,
                       const int* active_ring, int ring_size,
                       const double** dist, const int** ranking)
{
    if (!dist || !ranking || !active_ring) return 0.0;
    
    int* is_active = (int*)calloc(total_stations + 1, sizeof(int));
    if (!is_active) return 0.0;
    
    for (int i = 0; i < ring_size; ++i) {
        if (active_ring[i] > 0 && active_ring[i] <= total_stations)
            is_active[active_ring[i]] = 1;
    }

    double total = 0.0;

    for (int s = 1; s <= total_stations; ++s) {
        if (!is_active[s]) {
            const int* row = ranking[s - 1];
            if (!row) continue;
            int nearest = -1;
            for (int j = 0; j < total_stations; ++j) {
                int cand = row[j];
                if (cand >= 1 && cand <= total_stations && is_active[cand]) {
                    nearest = cand;
                    break;
                }
            }
            if (nearest > 0 && s >=1 && s <= total_stations && nearest >=1 && nearest <= total_stations)
                total += Cost_out_ring(alpha, dist[s - 1][nearest - 1]);
            else if (nearest > 0) {
                fprintf(stderr, "[COST] Warning: invalid nearest %d for station %d\n", nearest, s);
            }
        }
    }
    
    free(is_active);
    return total;
}

double Total_Cost_Individual(int alpha, const Individual* ind, int total_stations,
                             const double** dist, const int** ranking)
{
    if (!ind) return 1e18;
    
    return RingCostOnly(alpha, ind->active_ring, ind->ring_size, dist, total_stations)
         + OutRingCostOnly(alpha, total_stations, ind->active_ring, ind->ring_size, dist, ranking);
}

double* Total_Cost_Specie(int alpha, Individual* specie, int specie_size, int total_stations,
                          const double** dist, const int** ranking)
{
    double* costs = (double*)malloc(specie_size * sizeof(double));
    if (!costs) return NULL;

    for (int i = 0; i < specie_size; ++i) {
        if (!specie[i].active_ring) {
            // defensive
            fprintf(stderr, "[COST] Notice: individual %d has no active_ring, initializing\n", i);
            Individual_Init(&specie[i], 10);
        }
        if (specie[i].cached_cost < 1e17) {
            costs[i] = specie[i].cached_cost;
        } else {
            double c = Total_Cost_Individual(alpha, &specie[i], total_stations, dist, ranking);
            specie[i].cached_cost = c;
            costs[i] = c;
        }
    }

    return costs;
}
