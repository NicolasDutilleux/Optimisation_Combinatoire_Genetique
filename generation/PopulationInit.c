// generation/PopulationInit.c - Pure C implementation
// 
// Distribution IDENTIQUE au code de référence + 2-OPT sur une partie de la population
// pour maintenir la diversité
//
#include "PopulationInit.h"
#include "utils\Random.h"
#include "local_search\TwoOpt.h"
#include <stdlib.h>

Individual** Random_Generation(const Node* node_vector, int num_nodes,
                              int species_number, int individual_number,
                              int* out_species_count)
{
    Individual** species = (Individual**)malloc(species_number * sizeof(Individual*));
    if (!species) return NULL;
    
    int total_stations = num_nodes;
    
    for (int s = 0; s < species_number; ++s) {
        species[s] = (Individual*)malloc(individual_number * sizeof(Individual));
        if (!species[s]) {
            Free_Population(species, s, individual_number);
            return NULL;
        }
        
        for (int ind = 0; ind < individual_number; ++ind) {
            // Distribution identique au code de référence : RandInt(2, n)
            int ring_size = RandInt(2, total_stations);
            
            // Capacité = total_stations pour permettre croissance
            int capacity = total_stations;
            Individual_Init(&species[s][ind], capacity);
            
            // Préparer liste des IDs (1 à total_stations)
            int* all_ids = (int*)malloc(total_stations * sizeof(int));
            if (!all_ids) {
                Free_Population(species, s + 1, individual_number);
                return NULL;
            }
            
            for (int i = 0; i < total_stations; ++i) {
                all_ids[i] = i + 1;
            }
            
            // Fisher-Yates shuffle
            for (int i = total_stations - 1; i > 0; --i) {
                int j = RandInt(0, i);
                int tmp = all_ids[i];
                all_ids[i] = all_ids[j];
                all_ids[j] = tmp;
            }
            
            // Le nœud 1 (dépôt) toujours en premier
            species[s][ind].active_ring[0] = 1;
            species[s][ind].ring_size = 1;
            
            // Ajouter ring_size-1 nœuds depuis la liste mélangée
            int added = 0;
            for (int k = 0; k < total_stations && added < ring_size - 1; ++k) {
                if (all_ids[k] != 1) {
                    species[s][ind].active_ring[species[s][ind].ring_size++] = all_ids[k];
                    added++;
                }
            }
            
            free(all_ids);
        }
    }
    
    *out_species_count = species_number;
    return species;
}

// ============================================================================
// Appliquer 2-Opt sur une PARTIE de la population initiale
// ============================================================================
// Stratégie : Appliquer 2-Opt seulement sur certaines espèces pour garder
// de la diversité. Les espèces non-optimisées gardent des solutions "brutes"
// qui peuvent contenir des gènes utiles.
//
// fraction : proportion d'espèces à optimiser (ex: 0.2 = 20% = 1/5)
// ============================================================================
void Apply_TwoOpt_To_Population(Individual** species, int num_species, int pop_size,
                                 int alpha, const double** dist, const int** ranking)
{
    // Appliquer 2-Opt EXHAUSTIF sur 20% des espèces (1 espèce sur 5)
    int optimize_every = 5;  // 1 sur 5 = 20%
    
    for (int s = 0; s < num_species; ++s) {
        if (s % optimize_every == 0) {
            for (int i = 0; i < pop_size; ++i) {
                if (species[s][i].ring_size >= 3) {
                    // Utiliser 2-Opt EXHAUSTIF pour l'initialisation
                    TwoOptExhaustive(&species[s][i], alpha, dist, ranking);
                }
            }
        }
    }
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
