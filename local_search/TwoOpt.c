// local_search/TwoOpt.c - 2-opt local search implementation
//
// DEUX MODES DE 2-OPT :
// 1. EXHAUSTIF (pour initialisation) : Parcourt tout, jusqu'à convergence
// 2. ADAPTATIF (pour évolution) : Fenêtre aléatoire, nombre limité d'améliorations
//
#include "TwoOpt.h"
#include "utils\Random.h"
#include <stdio.h>

// =============================================================================
// 2-OPT ADAPTATIF (utilisé pendant l'évolution)
// =============================================================================
// Au lieu de parcourir TOUT le ring (O(n²)), on:
// 1. Choisit un point de départ aléatoire
// 2. Parcourt une fenêtre de taille proportionnelle au ring
// 3. S'arrête après un nombre limité d'améliorations
//
// IMPORTANT: On travaille sur des indices LINÉAIRES dans [start, end]
// pour éviter les bugs de wrap-around lors du reverse.
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

    // Paramètres adaptatifs
    int max_improvements = m / 4;
    if (max_improvements < 3) max_improvements = 3;
    if (max_improvements > 20) max_improvements = 20;
    
    // Taille de la fenêtre (portion du ring à explorer)
    int window_size = m / 2;
    if (window_size < 5) window_size = 5;
    if (window_size > m - 2) window_size = m - 2;
    
    // Point de départ aléatoire, s'assurer qu'on ne dépasse pas
    int max_start = m - window_size - 1;
    if (max_start < 0) max_start = 0;
    
    int start = (max_start > 0) ? RandInt(0, max_start) : 0;
    int end = start + window_size;
    if (end >= m) end = m - 1;
    
    int improvements = 0;
    
    // Parcourir la fenêtre [start, end] avec indices linéaires (pas de wrap)
    for (int i = start; i < end - 1 && improvements < max_improvements; ++i) {
        for (int j = i + 2; j <= end && improvements < max_improvements; ++j) {
            
            // Tous les accès sont dans [0, m-1] car i < end <= m-1 et j <= end
            int id_a = ind->active_ring[i];
            int id_b = ind->active_ring[i + 1];  // Safe: i < end-1, donc i+1 < end <= m-1
            int id_c = ind->active_ring[j];
            
            // Pour id_d, on utilise modulo car j+1 peut être = m
            int id_d = ind->active_ring[(j + 1) % m];

            double d0 = dist[id_a - 1][id_b - 1] + dist[id_c - 1][id_d - 1];
            double d1 = dist[id_a - 1][id_c - 1] + dist[id_b - 1][id_d - 1];
            
            if (d1 < d0) {
                // Reverse segment [i+1, j] - tous les indices sont dans [start, end]
                int a = i + 1;
                int b = j;
                while (a < b) {
                    int tmp = ind->active_ring[a];
                    ind->active_ring[a] = ind->active_ring[b];
                    ind->active_ring[b] = tmp;
                    a++;
                    b--;
                }
                improvements++;
            }
        }
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
        
        // i va de 0 à m-2 (pour que i+1 soit valide)
        for (int i = 0; i < m - 1; ++i) {
            // j va de i+2 à m-1
            for (int j = i + 2; j < m; ++j) {
                int id_a = ind->active_ring[i];
                int id_b = ind->active_ring[i + 1];  // Safe: i < m-1
                int id_c = ind->active_ring[j];
                int id_d = ind->active_ring[(j + 1) % m];  // Modulo pour le wrap

                double d0 = dist[id_a - 1][id_b - 1] + dist[id_c - 1][id_d - 1];
                double d1 = dist[id_a - 1][id_c - 1] + dist[id_b - 1][id_d - 1];
                
                if (d1 < d0) {
                    // Reverse segment [i+1, j]
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
