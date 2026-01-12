// cost/Cost.c
// Implementation of cost calculation functions
// Optimized: uses stack allocation for small arrays

#include "Cost.h"
#include "utils\StackConfig.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// =============================================================================
// BASIC COST FUNCTIONS
// =============================================================================

double Cost_station(int alpha, double distance) {
    return alpha * distance;
}

double Cost_out_ring(int alpha, double distance) {
    return (10.0 - alpha) * distance;
}

// =============================================================================
// RING COST CALCULATION
// =============================================================================

double RingCostOnly(
    int alpha,
    const int* active_ring, int ring_size,
    const double** dist, int dist_size)
{
    if (ring_size <= 1 || !dist || !active_ring) {
        return 0.0;
    }

    double cost = 0.0;

    for (int i = 0; i < ring_size; ++i) {
        int id_a = active_ring[i];
        int id_b = active_ring[(i + 1) % ring_size];

        if (id_a > 0 && id_a <= dist_size && id_b > 0 && id_b <= dist_size) {
            cost += alpha * dist[id_a - 1][id_b - 1];
        }
    }

    return cost;
}

// =============================================================================
// OUT-OF-RING COST CALCULATION (OPTIMIZED)
// =============================================================================

double OutRingCostOnly(
    int alpha, int total_stations,
    const int* active_ring, int ring_size,
    const double** dist, const int** ranking)
{
    if (!dist || !ranking || !active_ring || ring_size <= 0) {
        return 0.0;
    }

    // Use stack for small arrays, heap for large
    int stack_buffer[MAX_STACK_BUFFER_SIZE];
    int* is_active;
    int use_heap = (total_stations + 1 > MAX_STACK_BUFFER_SIZE);
    
    if (use_heap) {
        is_active = (int*)calloc(total_stations + 1, sizeof(int));
        if (!is_active) return 0.0;
    } else {
        is_active = stack_buffer;
        memset(is_active, 0, (total_stations + 1) * sizeof(int));
    }

    // Mark active stations
    for (int i = 0; i < ring_size; ++i) {
        int id = active_ring[i];
        if (id > 0 && id <= total_stations) {
            is_active[id] = 1;
        }
    }

    double total = 0.0;
    double out_weight = 10.0 - alpha;

    // For each station not in ring, find nearest ring station
    for (int station = 1; station <= total_stations; ++station) {
        if (is_active[station]) continue;

        const int* neighbors = ranking[station - 1];
        if (!neighbors) continue;

        // Find nearest ring station
        for (int j = 0; j < total_stations; ++j) {
            int candidate = neighbors[j];
            if (candidate >= 1 && candidate <= total_stations && is_active[candidate]) {
                total += out_weight * dist[station - 1][candidate - 1];
                break;
            }
        }
    }

    if (use_heap) free(is_active);
    return total;
}

// =============================================================================
// TOTAL COST CALCULATION
// =============================================================================

double Total_Cost_Individual(
    int alpha,
    const Individual* ind,
    int total_stations,
    const double** dist,
    const int** ranking)
{
    if (!ind || !ind->active_ring) {
        return 1e18;
    }

    double ring_cost = RingCostOnly(alpha, ind->active_ring, ind->ring_size, dist, total_stations);
    double out_cost = OutRingCostOnly(alpha, total_stations, ind->active_ring, ind->ring_size, dist, ranking);

    return ring_cost + out_cost;
}

double* Total_Cost_Specie(
    int alpha,
    Individual* specie, int specie_size,
    int total_stations,
    const double** dist,
    const int** ranking)
{
    double* costs = (double*)malloc(specie_size * sizeof(double));
    if (!costs) return NULL;

    for (int i = 0; i < specie_size; ++i) {
        if (!specie[i].active_ring) {
            Individual_Init(&specie[i], 10);
        }

        if (specie[i].cached_cost < 1e17) {
            costs[i] = specie[i].cached_cost;
        } else {
            costs[i] = Total_Cost_Individual(alpha, &specie[i], total_stations, dist, ranking);
            specie[i].cached_cost = costs[i];
        }
    }

    return costs;
}
