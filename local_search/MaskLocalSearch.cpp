#pragma once
#include "local_search/MaskLocalSearch.h"
#include "local_search/TwoOpt.h"
#include "cost/Cost.h"


void ImproveMaskLocal(
    Individual& ind,
    int alpha,
    int max_station_id,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking)
{
    const int N = max_station_id;

    while (true)
    {
        // Coût actuel
        double base_cost = Total_Cost_Individual(
            alpha,
            ind.ids,
            ind.mask,
            dist,
            ranking);

        double best_delta = 0.0;
        int    best_s = -1;
        bool   best_state = false; // futur état de la station

        // Compter le nombre de stations actives
        int active_count = 0;
        for (bool b : ind.mask) if (b) ++active_count;

        // On parcourt toutes les stations sauf le dépôt
        for (int s = 2; s <= N; ++s)
        {
            int idx = s - 1;
            bool curr = ind.mask[idx];

            // Option de sécurité : garder au moins 3 actives
            if (curr && active_count <= 3)
                continue;

            Individual cand = ind;
            bool new_state = !curr;
            cand.mask[idx] = new_state;

            if (new_state)
            {
                // OFF -> ON : activer s et l'insérer correctement dans le ring
                InsertStationBestPos(
                    cand.ids,
                    s,
                    dist);
            }
            // ON -> OFF : pas besoin de toucher cand.ids ; le ring ignore les inactives.

            // Réoptimiser le ring avec 2-opt
            TwoOptImproveAlpha(
                cand,
                alpha,
                dist,
                ranking);

            double new_cost = Total_Cost_Individual(
                alpha,
                cand.ids,
                cand.mask,
                dist,
                ranking);

            double delta = new_cost - base_cost;
            if (delta < best_delta - 1e-9)
            {
                best_delta = delta;
                best_s = s;
                best_state = new_state;
            }
        }

        // Aucun flip améliorant -> minimum local
        if (best_s == -1)
            break;

        // Appliquer le meilleur flip trouvé sur ind
        int idx = best_s - 1;
        bool old_state = ind.mask[idx];
        ind.mask[idx] = best_state;

        // Si on vient d'activer la station, l'insérer dans le ring
        if (!old_state && best_state)
        {
            InsertStationBestPos(
                ind.ids,
                best_s,
                dist);
        }

        // Réoptimiser définitivement le ring pour ce mask
        TwoOptImproveAlpha(
            ind,
            alpha,
            dist,
            ranking);

        // Et la boucle while recommence avec ce nouvel individu
    }
}