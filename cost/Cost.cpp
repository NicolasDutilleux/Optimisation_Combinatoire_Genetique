#include "Cost.h"
#include "utils/Distance.h" // Find_Nearest_Station
#include <algorithm>

double Cost_station(int alpha, double distance)
{
    return alpha * distance;
}

double Cost_out_ring(int alpha, double distance)
{
    return (10 - alpha) * distance;
}

double RingCostOnly(int alpha,
    const std::vector<int>& perm,
    const std::vector<bool>& mask,
    const std::vector<std::vector<double>>& dist)
{
    int n = static_cast<int>(perm.size());
    if (n <= 1) return 0.0;

    int first_idx = -1;
    for (int i = 0; i < n; ++i)
    {
        if (mask[perm[i] - 1]) { first_idx = i; break; }
    }
    if (first_idx == -1) return 1e18;

    double cost = 0.0;
    int prev_id = perm[first_idx];

    for (int step = 1; step < n; ++step)
    {
        int idx = (first_idx + step) % n;
        int id = perm[idx];
        if (!mask[id - 1]) continue;

        cost += Cost_station(alpha, dist[prev_id - 1][id - 1]);
        prev_id = id;
    }

    int first_id = perm[first_idx];
    if (prev_id != first_id)
        cost += Cost_station(alpha, dist[prev_id - 1][first_id - 1]);

    return cost;
}

double OutRingCostOnly(int alpha,
    const std::vector<bool>& mask,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector)
{
    int N = static_cast<int>(dist.size());
    double total = 0.0;

    for (int s = 1; s <= N; ++s)
    {
        if (!mask[s - 1])
        {
            int nearest = Find_Nearest_Station(s, mask, ranking_vector);
            total += Cost_out_ring(alpha, dist[s - 1][nearest - 1]);
        }
    }
    return total;
}

double Total_Cost_Individual(int alpha,
    const std::vector<int>& perm,
    const std::vector<bool>& mask,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector)
{
    return RingCostOnly(alpha, perm, mask, dist)
        + OutRingCostOnly(alpha, mask, dist, ranking_vector);
}

std::vector<double> Total_Cost_Specie(int alpha,
    std::vector<Individual>& specie,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking)
{
    std::vector<double> costs;
    costs.reserve(specie.size());

    for (auto& ind : specie)
        costs.push_back(Total_Cost_Individual(alpha, ind.ids, ind.mask, dist, ranking));

    return costs;
}

double RingCostActiveTour(int alpha,
    const std::vector<int>& active_ids,
    const std::vector<std::vector<double>>& dist)
{
    int m = static_cast<int>(active_ids.size());
    if (m <= 1) return 0.0;

    double cost = 0.0;
    for (int i = 0; i < m; ++i)
    {
        int a = active_ids[i];
        int b = active_ids[(i + 1) % m];
        cost += Cost_station(alpha, dist[a - 1][b - 1]);
    }
    return cost;
}
