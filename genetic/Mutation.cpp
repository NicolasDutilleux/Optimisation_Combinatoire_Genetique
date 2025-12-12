#include "genetic/Mutation.h"
#include "utils/Random.h"

#include <algorithm>
#include <numeric>

inline void Mutation_Swap_Simple(std::vector<int>& individual) {

    // Swap two random positions in the individual: No heuristic //

    int n = static_cast<int>(individual.size());
    int a = RandInt(1, n - 1); // preserve depot if that's the rule
    int b = RandInt(1, n - 1);
    std::swap(individual[a], individual[b]);
}

inline void Mutation_Inversion(std::vector<int>& individual) {

    // Invert a random segment of the individual : no heuristic but a lot of time segment are better //

    int n = static_cast<int>(individual.size());
    int a = RandInt(1, n - 1);
    int b = RandInt(1, n - 1);
    if (a > b) std::swap(a, b);
    std::reverse(individual.begin() + a, individual.begin() + b + 1);
}
inline void Mutation_Scramble(std::vector<int>& individual) {

    // Scramble a random segment of the individual : no heuristic but a lot of time segment are better //

    int n = static_cast<int>(individual.size());
    int a = RandInt(1, n - 1);
    int b = RandInt(1, n - 1);
    if (a > b) std::swap(a, b);
    std::shuffle(individual.begin() + a, individual.begin() + b + 1, GLOBAL_RNG());
}

inline void Mutation_Insert_Swap(std::vector<int>& individual) {

    // Remove an element at position a and insert it at position b  //
       /* Effect:
        -element at a is removed
        - elements a + 1 ... b shift left
        - val is inserted at position */

    int n = static_cast<int>(individual.size());
    int a = RandInt(1, n - 1);
    int b = RandInt(1, n - 1);
    if (a == b) return;
    int val = individual[a];
    if (a < b) {
        std::rotate(individual.begin() + a, individual.begin() + a + 1, individual.begin() + b + 1);
    }
    else {
        std::rotate(individual.begin() + b, individual.begin() + a, individual.begin() + a + 1);
    }
}

inline void Mutation_Deletion_Centroid(
    std::vector<int>& ids,
    std::vector<bool>& mask,
    const std::vector<Node>& nodes)
{

    // Deactivate one station based on centroid heuristic //


    if (std::count(mask.begin(), mask.end(), true) <= 2) return;

    // ---- 1. compute centroid of ACTIVE stations only ----
    double cx = 0.0, cy = 0.0;
    int activeCount = 0;

    for (int id = 1; id <= (int)mask.size(); id++) {
        if (mask[id - 1]) {
            const Node& nd = nodes[id - 1];
            cx += nd.x;
            cy += nd.y;
            activeCount++;
        }
    }

    cx /= activeCount;
    cy /= activeCount;

    // ---- 2. find ACTIVE station farthest from centroid (skip perm[0]) ----
    int best_index = -1;
    double best_dist = -1.0;

    for (int i = 1; i < (int)ids.size(); i++) {
        int station = ids[i];
        if (!mask[station - 1]) continue; // ignore inactive

        const Node& nd = nodes[station - 1];
        double dx = nd.x - cx;
        double dy = nd.y - cy;
        double d = dx * dx + dy * dy;

        if (d > best_dist) {
            best_dist = d;
            best_index = i;
        }
    }

    if (best_index <= 0) return;

    // ---- 3. deactivate this station ----
    int station_to_delete = ids[best_index];
    mask[station_to_delete - 1] = false;

    // no erase : perm remains same size
}

// Supprime aléatoirement entre 1 et max_del stations actives (hors dépôt),
// en gardant au moins min_active stations actives au total.
inline void Mutation_RandomMultiDeletion(
    const std::vector<int>& ids,
    std::vector<bool>& mask,
    int max_del,
    int min_active)
{
    int n = static_cast<int>(ids.size());

    // Nombre de stations actives actuelles
    int active_count = 0;
    for (bool b : mask) if (b) ++active_count;

    if (active_count <= min_active) return;

    // On ne peut pas supprimer plus que (active_count - min_active)
    int max_possible_del = std::min(max_del, active_count - min_active);
    if (max_possible_del <= 0) return;

    // Nombre effectif de deletions cette fois
    int k_del = RandInt(1, max_possible_del);

    // Construire la liste des positions (indices dans ids) qui sont actives, hors dépôt
    std::vector<int> active_positions;
    active_positions.reserve(n);

    for (int i = 1; i < n; ++i)  // i = 0 => dépôt, on ne le touche pas
    {
        int station_id = ids[i];         // l'ID de la station à cette position
        if (mask[station_id - 1])       // active ?
            active_positions.push_back(i);
    }

    if (active_positions.empty()) return;

    // On mélange et on choisit les k_del premières pour les désactiver
    std::shuffle(active_positions.begin(), active_positions.end(), GLOBAL_RNG());

    for (int i = 0; i < k_del && i < (int)active_positions.size(); ++i)
    {
        int pos = active_positions[i];
        int station_id = ids[pos];
        mask[station_id - 1] = false;
    }
}



inline void Mutation_Insertion_Heuristic(
    std::vector<int>& ids,
    std::vector<bool>& mask,
    int max_station_id,
    const std::vector<std::vector<double>>& dist)
{
    int n = static_cast<int>(ids.size());

    // 1. Pick a station that is currently NOT active
    int station_to_insert = RandInt(2, max_station_id);
    if (mask[station_to_insert - 1])
        return; // déjà active → rien à faire

    // 2. Compute best insertion position (min added cost) sur la permutation ids
    int best_pos = 1;
    double best_cost = 1e18;

    for (int pos = 1; pos < n; ++pos) {
        int prev = ids[pos - 1];
        int next = ids[pos];

        double added =
            dist[prev - 1][station_to_insert - 1] +
            dist[station_to_insert - 1][next - 1] -
            dist[prev - 1][next - 1];

        if (added < best_cost) {
            best_cost = added;
            best_pos = pos;
        }
    }

    // 3. Activer la station dans le mask
    mask[station_to_insert - 1] = true;

    // 4. Déplacer station_to_insert vers best_pos en préservant la permutation (swap simple)
    int old_pos = -1;
    for (int i = 0; i < n; ++i) {
        if (ids[i] == station_to_insert) {
            old_pos = i;
            break;
        }
    }
    if (old_pos == -1) return; // ne devrait jamais arriver

    // On rapproche station_to_insert de la "meilleure" position
    std::swap(ids[old_pos], ids[best_pos]);
}



Individual Mutations(
    int deletion_pourcentage,
    int multi_deletion_pourcentage,
    int swapping_pourcentage,
    int inversion_pourcentage,
    int scramble_pourcentage,
    int insertswap_pourcentage,
    const Individual& individual,
    int max_station_id,
    const std::vector<Node>& nodes,
    const std::vector<std::vector<double>>& dist)
{
    Individual ind = individual;

    // 1) Suppression "intelligente" : l'outlier vis-à-vis du centroïde
    if (RandInt(1, 100) <= deletion_pourcentage)
        Mutation_Deletion_Centroid(ind.ids, ind.mask, nodes);

    // 2) Multi-suppression random : pour vraiment explorer
    if (RandInt(1, 100) <= multi_deletion_pourcentage)
        Mutation_RandomMultiDeletion(ind.ids, ind.mask,
            /*max_del=*/3, /*min_active=*/3);

    // 3) Mutations sur la permutation
    if (RandInt(1, 100) <= swapping_pourcentage)
        Mutation_Swap_Simple(ind.ids);

    if (RandInt(1, 100) <= inversion_pourcentage)
        Mutation_Inversion(ind.ids);

    if (RandInt(1, 100) <= scramble_pourcentage)
        Mutation_Scramble(ind.ids);

    if (RandInt(1, 100) <= insertswap_pourcentage)
        Mutation_Insert_Swap(ind.ids);

    // 4) Insertion heuristique MAIS PAS obligatoire :
    //    => mets-la aussi sous proba, sinon tu annules souvent les deletions
    if (RandInt(1, 100) <= 30)  // par exemple 30% de chance
    {
        Mutation_Insertion_Heuristic(ind.ids,
            ind.mask,
            max_station_id,
            dist);
    }

    return ind;
}

