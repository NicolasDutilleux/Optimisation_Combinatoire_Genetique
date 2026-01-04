#include "local_search/TwoOpt.h"
#include "cost/Cost.h"
#include <algorithm>
#include <cmath>

// Extern for pre-computed distances
extern double Get_Distance(int i, int j);

void TwoOptImproveAlpha(
    Individual& ind,
    int alpha,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking)
{
    int m = static_cast<int>(ind.active_ring.size());
    if (m <= 3) return;
    if (dist.empty()) return;

    int max_iterations = 20;
    int iteration = 0;
    bool improved = true;

    while (improved && iteration < max_iterations)
    {
        ++iteration;
        improved = false;
        double best_delta = 0.0;
        int best_i = -1;
        int best_j = -1;

        // OPTIMIZED: Limit checks to first 1/3 of ring
        int check_limit = std::max(3, m / 3);

        for (int i = 0; i < check_limit && i < m - 2; ++i)
        {
            for (int j = i + 2; j < m; ++j)
            {
                int id_a = ind.active_ring[i];
                int id_b = ind.active_ring[i + 1];
                int id_c = ind.active_ring[j];
                int id_d = ind.active_ring[(j + 1) % m];

                if (id_a <= 0 || id_a > (int)dist.size() ||
                    id_b <= 0 || id_b > (int)dist.size() ||
                    id_c <= 0 || id_c > (int)dist.size() ||
                    id_d <= 0 || id_d > (int)dist.size())
                    continue;

                double old_cost = dist[id_a - 1][id_b - 1] + dist[id_c - 1][id_d - 1];
                double new_cost = dist[id_a - 1][id_c - 1] + dist[id_b - 1][id_d - 1];

                double delta = (new_cost - old_cost) * alpha;

                if (delta < best_delta - 1e-12)
                {
                    best_delta = delta;
                    best_i = i;
                    best_j = j;
                    improved = true;
                }
            }
        }

        if (!improved) break;

        std::reverse(ind.active_ring.begin() + best_i + 1,
                     ind.active_ring.begin() + best_j + 1);
    }
}

void InsertStationBestPos(
    std::vector<int>& ids,
    int station_id,
    const std::vector<std::vector<double>>& dist)
{
    const int n = static_cast<int>(ids.size());
    if (n <= 1) return;
    if (dist.empty()) return;
    if (station_id <= 0 || station_id > (int)dist.size()) return;

    int old_pos = -1;
    for (int i = 0; i < n; ++i)
    {
        if (ids[i] == station_id)
        {
            old_pos = i;
            break;
        }
    }
    if (old_pos == -1) return;

    int best_pos = 0;
    double best_delta = 1e18;

    for (int pos = 0; pos < n; ++pos)
    {
        int prev = ids[pos];
        int next = ids[(pos + 1) % n];

        if (prev <= 0 || prev > (int)dist.size() ||
            next <= 0 || next > (int)dist.size() ||
            station_id <= 0 || station_id > (int)dist.size())
            continue;

        double old_cost = dist[prev - 1][next - 1];
        double new_cost = dist[prev - 1][station_id - 1] + dist[station_id - 1][next - 1];

        double delta = new_cost - old_cost;
        if (delta < best_delta)
        {
            best_delta = delta;
            best_pos = pos;
        }
    }

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