// local_search/TwoOpt.c - 2-opt local search implementation
//
// DEUX MODES DE 2-OPT :
// 1. EXHAUSTIF (pour initialisation) : Continue jusqu'à convergence complète
// 2. ADAPTATIF (pour évolution) : Nombre d'itérations proportionnel à ring_size
//
#include "TwoOpt.h"
#include <stdio.h>

// =============================================================================
// 2-OPT ADAPTATIF (utilisé pendant l'évolution)
// =============================================================================
// Nombre d'itérations = ring_size / divisor (minimum 5, maximum 50)
// Cela permet :
//   - Petits rings (20 nœuds) : ~10 itérations (rapide)
//   - Grands rings (100 nœuds) : ~50 itérations (plus d'exploration)
//
void TwoOptImproveAlpha(Individual* ind, int alpha, const double** dist, const int** ranking)
{
    if (!ind || !dist) return;
    int m = ind->ring_size;
    if (m <= 3) return;

    // Validate indices
    for (int i = 0; i < m; ++i) {
        if (ind->active_ring[i] <= 0) return;
    }

    // Nombre d'itérations adaptatif : proportionnel à la taille du ring
    // ring_size / 2, avec min=5 et max=50
    int max_iterations = m / 2;
    if (max_iterations < 5) max_iterations = 5;
    if (max_iterations > 50) max_iterations = 50;
    
    int improved = 1;
    int iterations = 0;

    while (improved && iterations < max_iterations) {
        improved = 0;
        
        for (int i = 0; i < m - 1; ++i) {
            for (int j = i + 1; j < m; ++j) {
                int id_a = ind->active_ring[i];
                int id_b = ind->active_ring[(i + 1) % m];
                int id_c = ind->active_ring[j];
                int id_d = ind->active_ring[(j + 1) % m];

                double d0 = dist[id_a - 1][id_b - 1] + dist[id_c - 1][id_d - 1];
                double d1 = dist[id_a - 1][id_c - 1] + dist[id_b - 1][id_d - 1];
                
                if (d1 < d0) {
                    // Reverse segment from i+1 to j
                    int a = i + 1;
                    int b = j;
                    while (a < b) {
                        int tmp = ind->active_ring[a];
                        ind->active_ring[a] = ind->active_ring[b];
                        ind->active_ring[b] = tmp;
                        a++;
                        b--;
                    }
                    improved = 1;
                }
            }
        }
        iterations++;
    }
}

// =============================================================================
// 2-OPT EXHAUSTIF (utilisé pour l'initialisation de la population)
// =============================================================================
// Continue jusqu'à ce qu'aucune amélioration ne soit possible.
// Plus lent mais donne des solutions de meilleure qualité.
//
void TwoOptExhaustive(Individual* ind, int alpha, const double** dist, const int** ranking)
{
    if (!ind || !dist) return;
    int m = ind->ring_size;
    if (m <= 3) return;

    // Validate indices
    for (int i = 0; i < m; ++i) {
        if (ind->active_ring[i] <= 0) return;
    }

    int improved = 1;

    while (improved) {
        improved = 0;
        
        for (int i = 0; i < m - 1; ++i) {
            for (int j = i + 1; j < m; ++j) {
                int id_a = ind->active_ring[i];
                int id_b = ind->active_ring[(i + 1) % m];
                int id_c = ind->active_ring[j];
                int id_d = ind->active_ring[(j + 1) % m];

                double d0 = dist[id_a - 1][id_b - 1] + dist[id_c - 1][id_d - 1];
                double d1 = dist[id_a - 1][id_c - 1] + dist[id_b - 1][id_d - 1];
                
                if (d1 < d0) {
                    // Reverse segment from i+1 to j
                    int a = i + 1;
                    int b = j;
                    while (a < b) {
                        int tmp = ind->active_ring[a];
                        ind->active_ring[a] = ind->active_ring[b];
                        ind->active_ring[b] = tmp;
                        a++;
                        b--;
                    }
                    improved = 1;
                }
            }
        }
    }
}
