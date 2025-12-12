#include "local_search/TwoOpt.h"
#include "cost/Cost.h"

#include <algorithm>

void TwoOptImproveAlpha(Individual& ind,
    int alpha,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking)
{
    const int n = static_cast<int>(ind.ids.size());
    if (n <= 3) return;

    // ------------------------------------------------------------------
    // 1) Construire le tour des STATIONS ACTIVES dans l'ordre de ind.ids
    // ------------------------------------------------------------------
    std::vector<int> active_ids;
    active_ids.reserve(n);

    for (int pos = 0; pos < n; ++pos)
    {
        int id = ind.ids[pos];
        if (ind.mask[id - 1])   // station active ?
            active_ids.push_back(id);
    }

    const int m = static_cast<int>(active_ids.size());
    if (m <= 3) return; // trop peu de stations actives pour faire un vrai 2-opt

    // Option : s'assurer que le dépôt (id = 1) est en première position
    // Cela permet de garder une "référence" stable, comme dans le reste de ton code.
    {
        int depot_pos = -1;
        for (int i = 0; i < m; ++i)
        {
            if (active_ids[i] == 1)
            {
                depot_pos = i;
                break;
            }
        }
        if (depot_pos > 0)
        {
            // Rotation du tour de façon à avoir 1 au début
            std::rotate(active_ids.begin(),
                active_ids.begin() + depot_pos,
                active_ids.end());
        }
    }

    // ------------------------------------------------------------------
    // 2) Coût initial de l'anneau actif (uniquement la partie "ring")
    // ------------------------------------------------------------------
    double ring_cost = RingCostActiveTour(alpha, active_ids, dist);

    bool improved = true;

    // ------------------------------------------------------------------
    // 3) Boucle 2-opt avec formule Δ-coût locale
    //
    //    Complexité :
    //      - chaque passe parcourt toutes les paires (i, j) -> O(m^2)
    //      - le calcul de variation de coût est en O(1)
    //      - au lieu de recalculer tout le tour pour chaque candidat
    // ------------------------------------------------------------------
    while (improved)
    {
        improved = false;
        double best_delta = 0.0;
        int best_i = -1;
        int best_j = -1;

        // On fixe le sommet 0 (le dépôt) pour garder une "racine" stable.
        // i et j parcourent les autres positions du tour.
        for (int i = 1; i < m - 2; ++i)
        {
            for (int j = i + 1; j < m - 1; ++j)
            {
                // On coupe les arêtes (a-b) et (c-d) et on les remplace par (a-c) et (b-d).
                int a = active_ids[i - 1];
                int b = active_ids[i];
                int c = active_ids[j];
                int d = active_ids[(j + 1) % m];

                double old_cost =
                    dist[a - 1][b - 1] +
                    dist[c - 1][d - 1];

                double new_cost =
                    dist[a - 1][c - 1] +
                    dist[b - 1][d - 1];

                double delta = Cost_station(alpha, new_cost - old_cost);

                // On cherche la meilleure amélioration (delta < 0)
                if (delta < best_delta - 1e-12)
                {
                    best_delta = delta;
                    best_i = i;
                    best_j = j;
                    improved = true;
                }
            }
        }

        // Si aucune paire (i, j) n'améliore le tour, on est au minimum local
        if (!improved) break;

        // Appliquer la meilleure inversion trouvée
        std::reverse(active_ids.begin() + best_i,
            active_ids.begin() + best_j + 1);

        ring_cost += best_delta;
    }

    // ------------------------------------------------------------------
    // 4) Réinjecter le nouvel ordre actif dans ind.ids
    //
    //    On ne touche PAS au mask :
    //      - le pattern "actif / inactif" reste le même
    //    On remplace simplement, dans ind.ids, les IDs actifs dans le
    //    nouvel ordre 'active_ids', en conservant les villes inactives
    //    exactement aux mêmes positions qu'avant.
    //
    //    => Coût hors anneau inchangé (mask identique),
    //       seul le coût des arêtes entre stations actives est amélioré.
    // ------------------------------------------------------------------
    std::vector<int> new_ids = ind.ids;
    int idx_active = 0;

    for (int pos = 0; pos < n; ++pos)
    {
        int id = ind.ids[pos];
        if (ind.mask[id - 1])   // si cette position correspond à une station active
        {
            new_ids[pos] = active_ids[idx_active++];
        }
    }

    ind.ids.swap(new_ids);
}

// Insère une station donnée 'station_id' dans l'anneau ind.ids
// à la meilleure position (minimisation du surcoût sur le ring).
void InsertStationBestPos(
    std::vector<int>& ids,
    int station_id,
    const std::vector<std::vector<double>>& dist)
{
    const int n = static_cast<int>(ids.size());
    if (n <= 1) return;

    // 1) Trouver la position actuelle de station_id dans ids
    int old_pos = -1;
    for (int i = 0; i < n; ++i)
    {
        if (ids[i] == station_id)
        {
            old_pos = i;
            break;
        }
    }
    if (old_pos == -1) return; // sécurité : ne devrait pas arriver

    // 2) Choisir la meilleure position d'insertion (on laisse pos=0 pour le dépôt)
    int best_pos = 1;
    double best_delta = 1e18;

    for (int pos = 1; pos < n; ++pos)
    {
        int prev = ids[pos - 1];
        int next = ids[pos]; // on peut aussi considérer le tour fermé, mais ton ring
        // est ensuite retravaillé par TwoOptImproveAlpha.

        double old_cost =
            dist[prev - 1][next - 1];

        double new_cost =
            dist[prev - 1][station_id - 1] +
            dist[station_id - 1][next - 1];

        double delta = new_cost - old_cost;
        if (delta < best_delta)
        {
            best_delta = delta;
            best_pos = pos;
        }
    }

    // 3) Déplacer station_id de old_pos vers best_pos en conservant les autres
    if (old_pos < best_pos)
    {
        std::rotate(ids.begin() + old_pos,
            ids.begin() + old_pos + 1,
            ids.begin() + best_pos + 1);
    }
    else if (old_pos > best_pos)
    {
        std::rotate(ids.begin() + best_pos,
            ids.begin() + old_pos,
            ids.begin() + old_pos + 1);
    }
}