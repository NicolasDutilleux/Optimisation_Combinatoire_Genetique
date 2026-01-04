#include "utils/Distance.h"
#include <cmath>
#include <algorithm>
#include <unordered_set>

std::vector<std::vector<double>>
Compute_Distances_2DVector(const std::vector<Node>& node_vector)
{
    int n = static_cast<int>(node_vector.size());
    std::vector<std::vector<double>> dist(n, std::vector<double>(n));

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            double dx = node_vector[i].x - node_vector[j].x;
            double dy = node_vector[i].y - node_vector[j].y;
            dist[i][j] = std::sqrt(dx * dx + dy * dy);
        }
    }
    return dist;
}

std::vector<std::vector<int>>
Distance_Ranking_2DVector(std::vector<std::vector<double>> distance_vector)
{
    int n = static_cast<int>(distance_vector.size());
    std::vector<std::vector<int>> ranking(n, std::vector<int>(n));

    if (n == 0) return ranking;

    for (int i = 0; i < n; ++i)
    {
        // Create pairs of (distance, id) for efficient sorting
        std::vector<std::pair<double, int>> dist_id(n);
        for (int j = 0; j < n; ++j)
        {
            dist_id[j] = { distance_vector[i][j], j + 1 };
        }

        // OPTIMIZATION: Use std::sort (O(n log n)) instead of Bubble_Sort (O(n²))
        std::sort(dist_id.begin(), dist_id.end());

        for (int j = 0; j < n; ++j)
            ranking[i][j] = dist_id[j].second;
    }

    return ranking;
}

// OPTIMIZED: Use set lookup instead of nested loop
int Find_Nearest_Station(int city_id,
    const std::vector<int>& active_ring,
    const std::vector<std::vector<int>>& ranking_vector)
{
    if (city_id <= 0 || city_id > (int)ranking_vector.size())
        return -1;
    if (active_ring.empty())
        return -1;

    // Create a fast lookup set O(1) instead of O(M)
    std::unordered_set<int> ring_set(active_ring.begin(), active_ring.end());
    
    const std::vector<int>& row = ranking_vector[city_id - 1];

    for (int candidate_id : row)
    {
        if (ring_set.count(candidate_id) > 0)
            return candidate_id;
    }
    return -1;
}

std::vector<int>
Assign_Stations(int total_stations,
    const std::vector<int>& active_ring,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector)
{
    std::vector<int> assignment;

    if (dist.empty() || ranking_vector.empty())
        return assignment;

    // OPTIMIZED: Pre-compute ring membership as boolean array O(1) lookup
    std::vector<bool> is_in_ring(total_stations + 1, false);
    for (int ring_id : active_ring)
    {
        if (ring_id > 0 && ring_id <= total_stations)
            is_in_ring[ring_id] = true;
    }

    for (int s = 1; s <= total_stations && s <= (int)dist.size(); ++s)
    {
        if (!is_in_ring[s])  // O(1) lookup instead of nested loop
        {
            int nearest = Find_Nearest_Station(s, active_ring, ranking_vector);
            if (nearest > 0)
            {
                assignment.push_back(s);
                assignment.push_back(nearest);
            }
        }
    }
    return assignment;
}
