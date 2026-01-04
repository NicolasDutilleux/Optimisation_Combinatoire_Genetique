#include "genetic/Mutation.h"
#include "utils/Random.h"
#include <algorithm>
#include <iostream>
#include <unordered_set>

void Mutation_Add_Node(
    std::vector<int>& active_ring,
    int total_stations,
    const std::vector<std::vector<double>>& dist)
{
    // OPTIMIZATION: Use set for O(1) membership check
    std::unordered_set<int> ring_set(active_ring.begin(), active_ring.end());
    
    std::vector<int> inactive;
    for (int s = 1; s <= total_stations; ++s)
    {
        if (ring_set.count(s) == 0)  // O(1) instead of loop
            inactive.push_back(s);
    }

    if (inactive.empty()) return;

    int station_to_add = inactive[RandInt(0, static_cast<int>(inactive.size()) - 1)];

    int m = static_cast<int>(active_ring.size());
    if (m == 0)
    {
        active_ring.push_back(station_to_add);
        return;
    }

    int best_pos = 0;
    double best_cost = 1e18;

    for (int pos = 0; pos < m; ++pos)
    {
        int id_prev = active_ring[pos];
        int id_next = active_ring[(pos + 1) % m];

        if (id_prev <= 0 || id_prev > (int)dist.size() || id_next <= 0 || id_next > (int)dist.size())
            continue;

        double old_dist = dist[id_prev - 1][id_next - 1];
        double new_dist = dist[id_prev - 1][station_to_add - 1]
                        + dist[station_to_add - 1][id_next - 1];

        double added_cost = new_dist - old_dist;
        if (added_cost < best_cost)
        {
            best_cost = added_cost;
            best_pos = pos;
        }
    }

    active_ring.insert(active_ring.begin() + best_pos + 1, station_to_add);
}

void Mutation_Remove_Node(
    std::vector<int>& active_ring,
    int min_ring_size)
{
    if (static_cast<int>(active_ring.size()) <= min_ring_size)
        return;

    int idx = RandInt(0, static_cast<int>(active_ring.size()) - 1);
    active_ring.erase(active_ring.begin() + idx);
}

void Mutation_Swap_Simple(std::vector<int>& active_ring)
{
    int m = static_cast<int>(active_ring.size());
    if (m < 2) return;

    int a = RandInt(0, m - 1);
    int b = RandInt(0, m - 1);
    std::swap(active_ring[a], active_ring[b]);
}

void Mutation_Inversion(std::vector<int>& active_ring)
{
    int m = static_cast<int>(active_ring.size());
    if (m < 2) return;

    int a = RandInt(0, m - 1);
    int b = RandInt(0, m - 1);
    if (a > b) std::swap(a, b);
    std::reverse(active_ring.begin() + a, active_ring.begin() + b + 1);
}

void Mutation_Scramble(std::vector<int>& active_ring)
{
    int m = static_cast<int>(active_ring.size());
    if (m < 2) return;

    int a = RandInt(0, m - 1);
    int b = RandInt(0, m - 1);
    if (a > b) std::swap(a, b);
    std::shuffle(active_ring.begin() + a, active_ring.begin() + b + 1, GLOBAL_RNG());
}

Individual Mutations(
    int add_percentage,
    int remove_percentage,
    int swap_percentage,
    int inversion_percentage,
    int scramble_percentage,
    const Individual& individual,
    int total_stations,
    const std::vector<std::vector<double>>& dist)
{
    Individual ind = individual;

    if (RandInt(1, 100) <= add_percentage)
        Mutation_Add_Node(ind.active_ring, total_stations, dist);

    if (RandInt(1, 100) <= remove_percentage)
        Mutation_Remove_Node(ind.active_ring);

    if (RandInt(1, 100) <= swap_percentage)
        Mutation_Swap_Simple(ind.active_ring);

    if (RandInt(1, 100) <= inversion_percentage)
        Mutation_Inversion(ind.active_ring);

    if (RandInt(1, 100) <= scramble_percentage)
        Mutation_Scramble(ind.active_ring);

    return ind;
}

