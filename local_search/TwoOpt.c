// local_search/TwoOpt.c - 2-opt local search implementation
#include "TwoOpt.h"
#include <stdio.h>

void TwoOptImproveAlpha(Individual* ind, int alpha, const double** dist, const int** ranking)
{
    if (!ind || !dist) return;
    int m = ind->ring_size;
    if (m <= 3) return;

    // Validate indices
    for (int i = 0; i < m; ++i) {
        if (ind->active_ring[i] <= 0) return;
    }

    int max_iterations = 10;
    int improved = 1;

    while (improved && max_iterations-- > 0) {
        improved = 0;
        double best_delta = 0.0;
        int best_i = -1, best_j = -1;

        for (int i = 0; i < m - 2; ++i) {
            for (int j = i + 2; j < m; ++j) {
                int id_a = ind->active_ring[i];
                int id_b = ind->active_ring[i + 1];
                int id_c = ind->active_ring[j];
                int id_d = ind->active_ring[(j + 1) % m];

                // Current cost of edges (a-b) and (c-d)
                double old_cost = dist[id_a - 1][id_b - 1] + dist[id_c - 1][id_d - 1];
                // New cost if we reverse segment: edges become (a-c) and (b-d)
                double new_cost = dist[id_a - 1][id_c - 1] + dist[id_b - 1][id_d - 1];
                
                // Delta < 0 means improvement (lower cost)
                double delta = new_cost - old_cost;

                if (delta < best_delta - 1e-9) {
                    best_delta = delta;
                    best_i = i;
                    best_j = j;
                    improved = 1;
                }
            }
        }

        if (!improved) break;

        // Reverse segment from best_i+1 to best_j
        int a = best_i + 1;
        int b = best_j;
        while (a < b) {
            int tmp = ind->active_ring[a];
            ind->active_ring[a] = ind->active_ring[b];
            ind->active_ring[b] = tmp;
            a++;
            b--;
        }
    }
}
