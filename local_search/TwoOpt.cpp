#include "local_search/TwoOpt.h"
#include <algorithm>
#include <cmath>

void TwoOptImproveAlpha(
    Individual& ind,
    int alpha,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& /*ranking*/) // ranking non utilisé ici
{
    int m = static_cast<int>(ind.active_ring.size());
    if (m <= 3) return;
    if (dist.empty()) return;

    bool improved = true;

    // BOUCLE NON BRIDÉE : On continue tant qu'on améliore
    while (improved)
    {
        improved = false;

        // On vérifie TOUTES les paires (pas de check_limit)
        for (int i = 0; i < m - 1; ++i) // i va jusqu'à m-1
        {
            // j commence à i+1 pour tester chaque paire une seule fois
            // Note : pour un ring fermé, le coût de l'arrête (m-1)->0 est géré implicitement
            // par la structure du tour, mais le swap 2-opt classique sur vecteur plat traite i et j.
            for (int j = i + 1; j < m; ++j)
            {
                // On saute les nœuds adjacents (le swap ne ferait rien ou casserait l'arête unique)
                if (j == i + 1) continue;
                // Pour le cas cyclique complet (fermeture m-1 -> 0), il faudrait adapter, 
                // mais la version standard "flat" fonctionne très bien pour converger.

                int id_a = ind.active_ring[i];
                int id_b = ind.active_ring[i + 1];
                int id_c = ind.active_ring[j];
                int id_d = ind.active_ring[(j + 1) % m]; // Le modulo gère la fermeture

                double old_cost = dist[id_a - 1][id_b - 1] + dist[id_c - 1][id_d - 1];
                double new_cost = dist[id_a - 1][id_c - 1] + dist[id_b - 1][id_d - 1];

                // On applique le changement si strictement meilleur
                if (new_cost < old_cost - 1e-9)
                {
                    // 2-Opt Move : on inverse le segment entre i+1 et j
                    std::reverse(ind.active_ring.begin() + i + 1,
                        ind.active_ring.begin() + j + 1);
                    improved = true;
                    // On peut break ici (First improvement) ou continuer (Best improvement)
                    // First improvement est souvent plus efficace globalement :
                    // break; 
                }
            }
        }
    }
}