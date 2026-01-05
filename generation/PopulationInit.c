// generation/PopulationInit.c - Pure C implementation (fixed all_ids bounds)
#include "PopulationInit.h"
#include "utils\Random.h"
#include <stdlib.h>

Individual** Random_Generation(const Node* node_vector, int num_nodes,
                              int species_number, int individual_number,
                              int* out_species_count)
{
    Individual** species = (Individual**)malloc(species_number * sizeof(Individual*));
    if (!species) return NULL;
    
    int total_stations = num_nodes;
    int max_initial_ring = (20 < (total_stations / 5)) ? 20 : (total_stations / 5);
    if (max_initial_ring < 3) max_initial_ring = 3;
    
    for (int s = 0; s < species_number; ++s) {
        species[s] = (Individual*)malloc(individual_number * sizeof(Individual));
        if (!species[s]) {
            Free_Population(species, s, individual_number);
            return NULL;
        }
        
        for (int ind = 0; ind < individual_number; ++ind) {
            // Initialize the Individual in-place to avoid temporary ownership issues
            Individual_Init(&species[s][ind], max_initial_ring);
            
            int ring_size = RandInt(3, (max_initial_ring < total_stations) ? max_initial_ring : total_stations);
            
            // prepare list of candidate ids excluding station 1: ids 2..total_stations
            int inactive_count_capacity = (total_stations > 1) ? (total_stations - 1) : 0;
            int* all_ids = NULL;
            if (inactive_count_capacity > 0) {
                all_ids = (int*)malloc(inactive_count_capacity * sizeof(int));
                if (!all_ids) {
                    Free_Population(species, s + 1, individual_number);
                    return NULL;
                }
            }
            
            for (int i = 0; i < inactive_count_capacity; ++i) {
                all_ids[i] = i + 2; // 2..total_stations
            }
            
            for (int i = inactive_count_capacity - 1; i > 0; --i) {
                int j = RandInt(0, i);
                int tmp = all_ids[i];
                all_ids[i] = all_ids[j];
                all_ids[j] = tmp;
            }
            
            // Ensure station 1 is present
            if (species[s][ind].ring_capacity > 0) {
                species[s][ind].active_ring[species[s][ind].ring_size++] = 1;
            }
            
            // Fill remaining from shuffled all_ids
            for (int k = 0; k < ring_size - 1 && k < inactive_count_capacity; ++k) {
                if (species[s][ind].ring_size < species[s][ind].ring_capacity) {
                    species[s][ind].active_ring[species[s][ind].ring_size++] = all_ids[k];
                }
            }
            
            if (all_ids) free(all_ids);
        }
    }
    
    *out_species_count = species_number;
    return species;
}

void Free_Population(Individual** species, int num_species, int ind_per_species)
{
    if (!species) return;
    
    for (int s = 0; s < num_species; ++s) {
        if (species[s]) {
            for (int i = 0; i < ind_per_species; ++i) {
                Individual_Free(&species[s][i]);
            }
            free(species[s]);
        }
    }
    free(species);
}
