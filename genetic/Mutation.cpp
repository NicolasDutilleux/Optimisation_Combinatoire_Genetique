#include "genetic/Mutation.h"
#include "utils/Random.h"
#include <algorithm>
#include <vector>
#include <unordered_set>

void Mutation_Add_Node(
    std::vector<int>& active_ring,
    int total_stations,
    const std::vector<std::vector<double>>& /*dist*/) // dist n'est plus utilisé !
{
    // 1. Identifier les nœuds inactifs (Optimisé avec un set)
    std::unordered_set<int> ring_set(active_ring.begin(), active_ring.end());
    std::vector<int> inactive;
    inactive.reserve(total_stations - active_ring.size());

    for (int s = 1; s <= total_stations; ++s)
    {
        if (ring_set.find(s) == ring_set.end())
            inactive.push_back(s);
    }

    if (inactive.empty()) return;

    // 2. Choisir un nœud inactif au hasard
    int station_to_add = inactive[RandInt(0, static_cast<int>(inactive.size()) - 1)];

    // 3. Insérer à une position ALÉATOIRE (C'est la clé !)
    // On ne calcule plus le coût. Le 2-Opt passera derrière pour réparer.
    int m = static_cast<int>(active_ring.size());
    int pos = RandInt(1, m); // 1 à m pour ne pas déplacer le dépôt (index 0) si on veut le garder fixe
    // Si le dépôt n'est pas fixe, 0 à m est ok.
    // Supposons index 0 = dépôt fixe :

    if (m == 0) active_ring.push_back(station_to_add);
    else active_ring.insert(active_ring.begin() + pos, station_to_add);
}

void Mutation_Remove_Node(std::vector<int>& active_ring, int min_ring_size)
{
    if (static_cast<int>(active_ring.size()) <= min_ring_size) return;
    // On évite de supprimer le dépôt (index 0)
    int idx = RandInt(1, static_cast<int>(active_ring.size()) - 1);
    active_ring.erase(active_ring.begin() + idx);
}

void Mutation_Swap_Simple(std::vector<int>& active_ring)
{
    int m = static_cast<int>(active_ring.size());
    if (m < 3) return;
    int a = RandInt(1, m - 1); // Pas le dépôt
    int b = RandInt(1, m - 1);
    std::swap(active_ring[a], active_ring[b]);
}

void Mutation_Inversion(std::vector<int>& active_ring)
{
    int m = static_cast<int>(active_ring.size());
    if (m < 3) return;
    int a = RandInt(1, m - 1);
    int b = RandInt(1, m - 1);
    if (a > b) std::swap(a, b);
    std::reverse(active_ring.begin() + a, active_ring.begin() + b + 1);
}

void Mutation_Scramble(std::vector<int>& active_ring)
{
    int m = static_cast<int>(active_ring.size());
    if (m < 3) return;
    int a = RandInt(1, m - 1);
    int b = RandInt(1, m - 1);
    if (a > b) std::swap(a, b);
    std::shuffle(active_ring.begin() + a, active_ring.begin() + b + 1, GLOBAL_RNG());
}

Individual Mutations(
    int add_pct, int rem_pct, int swap_pct, int inv_pct, int scr_pct,
    const Individual& individual,
    int total_stations,
    const std::vector<std::vector<double>>& dist)
{
    Individual ind = individual;

    // On applique chaque mutation selon sa probabilité
    if (RandInt(1, 100) <= add_pct)  Mutation_Add_Node(ind.active_ring, total_stations, dist);
    if (RandInt(1, 100) <= rem_pct)  Mutation_Remove_Node(ind.active_ring);
    if (RandInt(1, 100) <= swap_pct) Mutation_Swap_Simple(ind.active_ring);

    // Inversion et Scramble sont moins critiques si le 2-Opt fait son travail,
    // mais on peut les garder pour la diversité.
    if (RandInt(1, 100) <= inv_pct)  Mutation_Inversion(ind.active_ring);

    return ind;
}