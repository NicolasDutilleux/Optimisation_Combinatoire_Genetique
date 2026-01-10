// cost/Cost.h
// Cost calculation functions for ring optimization
//
// The cost model has two components:
// 1. Ring cost: cost of edges within the active ring
// 2. Out-of-ring cost: cost for stations not in the ring to reach nearest ring station

#ifndef COST_H
#define COST_H

#include "core\Individual.h"

// =============================================================================
// BASIC COST FUNCTIONS
// =============================================================================

// Cost for an edge on the ring
// Returns: alpha * distance
double Cost_station(int alpha, double distance);

// Cost for connecting an out-of-ring station to the ring
// Returns: (10 - alpha) * distance
double Cost_out_ring(int alpha, double distance);

// =============================================================================
// RING COST CALCULATION
// =============================================================================

// Calculate total cost of edges in the ring
//
// Parameters:
//   alpha       - Weight for ring edges
//   active_ring - Array of station IDs in the ring (1-indexed)
//   ring_size   - Number of stations in ring
//   dist        - Distance matrix (0-indexed)
//   dist_size   - Size of distance matrix
//
// Returns: Sum of Cost_station for all consecutive pairs in ring
//
double RingCostOnly(
    int alpha,
    const int* active_ring, int ring_size,
    const double** dist, int dist_size
);

// =============================================================================
// OUT-OF-RING COST CALCULATION
// =============================================================================

// Calculate total cost for stations not in the ring
//
// For each station not in the ring, finds the nearest ring station
// and adds Cost_out_ring for that connection.
//
// Parameters:
//   alpha          - Weight for out-of-ring connections
//   total_stations - Total number of stations
//   active_ring    - Array of station IDs in the ring (1-indexed)
//   ring_size      - Number of stations in ring
//   dist           - Distance matrix (0-indexed)
//   ranking        - Sorted neighbors matrix (0-indexed)
//
// Returns: Sum of Cost_out_ring for all non-ring stations
//
double OutRingCostOnly(
    int alpha, int total_stations,
    const int* active_ring, int ring_size,
    const double** dist, const int** ranking
);

// =============================================================================
// TOTAL COST CALCULATION
// =============================================================================

// Calculate total cost for a single individual
// Returns: RingCostOnly + OutRingCostOnly
//
double Total_Cost_Individual(
    int alpha,
    const Individual* ind,
    int total_stations,
    const double** dist,
    const int** ranking
);

// Calculate costs for all individuals in a species
// Returns: Newly allocated array of costs (caller must free)
//          Returns NULL on error
//
double* Total_Cost_Specie(
    int alpha,
    Individual* specie, int specie_size,
    int total_stations,
    const double** dist,
    const int** ranking
);

#endif // COST_H

