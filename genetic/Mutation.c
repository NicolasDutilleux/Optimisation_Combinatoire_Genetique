// genetic/Mutation.c - Optimized with stack allocation
#include "Mutation.h"
#include "utils\Random.h"
#include <stdlib.h>
#include <string.h>

#define MAX_STACK_SIZE 256

void Mutation_Add_Node(Individual* ind, int total_stations, const double** dist)
{
    if (!ind || !dist || total_stations <= 0) return;
    
    // Use stack for small arrays
    int stack_active[MAX_STACK_SIZE];
    int stack_inactive[MAX_STACK_SIZE];
    int* is_active;
    int* inactive;
    int use_heap = (total_stations + 1 > MAX_STACK_SIZE);
    
    if (use_heap) {
        is_active = (int*)calloc(total_stations + 1, sizeof(int));
        inactive = (int*)malloc(total_stations * sizeof(int));
        if (!is_active || !inactive) {
            if (is_active) free(is_active);
            if (inactive) free(inactive);
            return;
        }
    } else {
        is_active = stack_active;
        inactive = stack_inactive;
        memset(is_active, 0, (total_stations + 1) * sizeof(int));
    }
    
    // Mark active stations
    for (int i = 0; i < ind->ring_size; ++i) {
        int id = ind->active_ring[i];
        if (id > 0 && id <= total_stations)
            is_active[id] = 1;
    }
    
    // Find inactive stations
    int inactive_count = 0;
    for (int s = 1; s <= total_stations; ++s) {
        if (!is_active[s]) {
            inactive[inactive_count++] = s;
        }
    }
    
    if (inactive_count == 0) {
        if (use_heap) { free(is_active); free(inactive); }
        return;
    }
    
    int station_to_add = inactive[RandInt(0, inactive_count - 1)];
    
    if (ind->ring_size == 0) {
        if (ind->ring_capacity > 0) {
            ind->active_ring[0] = station_to_add;
            ind->ring_size = 1;
        }
        if (use_heap) { free(is_active); free(inactive); }
        return;
    }
    
    // Find best insertion position
    int best_pos = 0;
    double best_cost = 1e18;
    
    for (int pos = 0; pos < ind->ring_size; ++pos) {
        int id_prev = ind->active_ring[pos];
        int id_next = ind->active_ring[(pos + 1) % ind->ring_size];
        
        if (id_prev > 0 && id_next > 0 && id_prev <= total_stations && id_next <= total_stations) {
            double old_dist = dist[id_prev - 1][id_next - 1];
            double new_dist = dist[id_prev - 1][station_to_add - 1]
                            + dist[station_to_add - 1][id_next - 1];
            double cost_delta = new_dist - old_dist;
            
            if (cost_delta < best_cost) {
                best_cost = cost_delta;
                best_pos = pos;
            }
        }
    }
    
    // Insert station
    if (ind->ring_size < ind->ring_capacity) {
        for (int i = ind->ring_size; i > best_pos + 1; --i) {
            ind->active_ring[i] = ind->active_ring[i - 1];
        }
        ind->active_ring[best_pos + 1] = station_to_add;
        ind->ring_size++;
    }
    
    if (use_heap) { free(is_active); free(inactive); }
}

void Mutation_Remove_Node(Individual* ind, int min_ring_size)
{
    if (!ind || ind->ring_size <= min_ring_size) return;
    
    int idx = RandInt(0, ind->ring_size - 1);
    
    for (int i = idx; i < ind->ring_size - 1; ++i) {
        ind->active_ring[i] = ind->active_ring[i + 1];
    }
    ind->ring_size--;
}

void Mutation_Swap_Simple(Individual* ind)
{
    if (!ind || ind->ring_size < 2) return;
    
    int a = RandInt(0, ind->ring_size - 1);
    int b = RandInt(0, ind->ring_size - 1);
    
    int tmp = ind->active_ring[a];
    ind->active_ring[a] = ind->active_ring[b];
    ind->active_ring[b] = tmp;
}

void Mutation_Inversion(Individual* ind)
{
    if (!ind || ind->ring_size < 2) return;
    
    int a = RandInt(0, ind->ring_size - 1);
    int b = RandInt(0, ind->ring_size - 1);
    if (a > b) { int tmp = a; a = b; b = tmp; }
    
    while (a < b) {
        int tmp = ind->active_ring[a];
        ind->active_ring[a] = ind->active_ring[b];
        ind->active_ring[b] = tmp;
        a++; b--;
    }
}

void Mutation_Scramble(Individual* ind)
{
    if (!ind || ind->ring_size < 2) return;
    
    int a = RandInt(0, ind->ring_size - 1);
    int b = RandInt(0, ind->ring_size - 1);
    if (a > b) { int tmp = a; a = b; b = tmp; }
    
    for (int i = a; i <= b; ++i) {
        int j = RandInt(a, b);
        int tmp = ind->active_ring[i];
        ind->active_ring[i] = ind->active_ring[j];
        ind->active_ring[j] = tmp;
    }
}

void Mutations(Individual* out_individual,
               int add_percentage, int remove_percentage, int swap_percentage,
               int inversion_percentage, int scramble_percentage,
               const Individual* individual, int total_stations, const double** dist)
{
    if (!out_individual || !individual) return;
    
    Individual_Copy(out_individual, individual);
    
    if (RandInt(1, 100) <= add_percentage) {
        Mutation_Add_Node(out_individual, total_stations, dist);
    }
    if (RandInt(1, 100) <= remove_percentage) {
        Mutation_Remove_Node(out_individual, 3);
    }
    if (RandInt(1, 100) <= swap_percentage) {
        Mutation_Swap_Simple(out_individual);
    }
    if (RandInt(1, 100) <= inversion_percentage) {
        Mutation_Inversion(out_individual);
    }
    if (RandInt(1, 100) <= scramble_percentage) {
        Mutation_Scramble(out_individual);
    }
    
    out_individual->cached_cost = 1e18;
}
