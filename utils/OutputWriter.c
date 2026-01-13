// utils/OutputWriter.c
// Implementation of solution file writer

#define _CRT_SECURE_NO_WARNINGS
#include "OutputWriter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void WriteSolutionToFile(
    const Individual* best_individual,
    int total_stations,
    int alpha,
    double best_cost,
    const int** dist_ranking)
{
    if (!best_individual || !best_individual->active_ring) {
        fprintf(stderr, "ERROR: Cannot write solution (invalid individual)\n");
        return;
    }

    // Open output file
    FILE* f = fopen("Genetic_Solution.txt", "w");
    if (!f) {
        fprintf(stderr, "ERROR: Cannot create Genetic_Solution.txt\n");
        return;
    }

    // Write header
    fprintf(f, "DIMENSION: %d\n", total_stations);
    fprintf(f, "ALPHA: %d\n", alpha);
    fprintf(f, "BEST_COST: %.2f\n", best_cost);
    fprintf(f, "\n");

    // Write ring (tour order)
    fprintf(f, "RING:\n");
    for (int i = 0; i < best_individual->ring_size; i++) {
        fprintf(f, "%d", best_individual->active_ring[i]);
        if (i < best_individual->ring_size - 1) {
            fprintf(f, " ");
        }
    }
    // Add depot at end to close the loop
    if (best_individual->ring_size > 0) {
        fprintf(f, " %d", best_individual->active_ring[0]);
    }
    fprintf(f, "\n\n");

    // Build is_active map
    int* is_active = (int*)calloc(total_stations + 1, sizeof(int));
    if (!is_active) {
        fprintf(stderr, "ERROR: Memory allocation failed\n");
        fclose(f);
        return;
    }

    for (int i = 0; i < best_individual->ring_size; i++) {
        int id = best_individual->active_ring[i];
        if (id > 0 && id <= total_stations) {
            is_active[id] = 1;
        }
    }

    // Write assignments (non-ring nodes -> nearest ring node)
    fprintf(f, "ASSIGNMENTS:\n");
    int has_assignments = 0;
    
    for (int station = 1; station <= total_stations; station++) {
        if (is_active[station]) continue;  // Skip ring nodes
        
        // Find nearest ring node using distance ranking
        const int* neighbors = dist_ranking[station - 1];
        if (!neighbors) continue;
        
        int nearest_ring = -1;
        for (int j = 0; j < total_stations; j++) {
            int candidate = neighbors[j];
            if (candidate >= 1 && candidate <= total_stations && is_active[candidate]) {
                nearest_ring = candidate;
                break;
            }
        }
        
        if (nearest_ring > 0) {
            fprintf(f, "%d -> %d\n", station, nearest_ring);
            has_assignments = 1;
        }
    }
    
    if (!has_assignments) {
        fprintf(f, "(no assignments - all nodes in ring)\n");
    }

    free(is_active);
    fclose(f);

    printf("\n[OUTPUT] Solution saved to Genetic_Solution.txt\n");
}
