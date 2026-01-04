#include "Cost.h"
#include "utils/Distance.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <thread>

double Cost_station(int alpha, double distance)
{
    return alpha * distance;
}

double Cost_out_ring(int alpha, double distance)
{
    return (10.0 - alpha) * distance;
}

double RingCostOnly(int alpha,
    const std::vector<int>& active_ring,
    const std::vector<std::vector<double>>& dist)
{
    int m = static_cast<int>(active_ring.size());
    if (m <= 1) return 0.0;
    if (dist.empty()) return 0.0;

    double cost = 0.0;
    for (int i = 0; i < m; ++i)
    {
        int id_a = active_ring[i];
        int id_b = active_ring[(i + 1) % m];
        
        if (id_a <= 0 || id_a > (int)dist.size() || id_b <= 0 || id_b > (int)dist.size())
            continue;
            
        cost += Cost_station(alpha, dist[id_a - 1][id_b - 1]);
    }
    return cost;
}

double OutRingCostOnly(int alpha,
    int total_stations,
    const std::vector<int>& active_ring,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector)
{
    if (dist.empty() || ranking_vector.empty()) return 0.0;
    
    std::vector<bool> is_active(total_stations + 1, false);
    for (int ring_id : active_ring)
    {
        if (ring_id > 0 && ring_id <= total_stations)
            is_active[ring_id] = true;
    }

    double total = 0.0;

    for (int s = 1; s <= total_stations; ++s)
    {
        if (!is_active[s])
        {
            const std::vector<int>& row = ranking_vector[s - 1];
            int nearest = -1;
            for (int cand : row)
            {
                if (cand >= 1 && cand <= total_stations && is_active[cand])
                {
                    nearest = cand;
                    break;
                }
            }
            if (nearest > 0 && s <= (int)dist.size() && nearest <= (int)dist.size())
                total += Cost_out_ring(alpha, dist[s - 1][nearest - 1]);
        }
    }
    return total;
}

double Total_Cost_Individual(int alpha,
    const Individual& ind,
    int total_stations,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector)
{
    return RingCostOnly(alpha, ind.active_ring, dist)
        + OutRingCostOnly(alpha, total_stations, ind.active_ring, dist, ranking_vector);
}

std::vector<double> Total_Cost_Specie(int alpha,
    std::vector<Individual>& specie,
    int total_stations,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking)
{
    int popsize = static_cast<int>(specie.size());
    std::vector<double> costs(popsize);

    // Parallel evaluation using threads
    unsigned int hw = std::thread::hardware_concurrency();
    if (hw == 0) hw = 4;
    int threads = std::min<unsigned int>(hw, popsize);
    int chunk = (popsize + threads - 1) / threads;

    std::vector<std::thread> workers;
    workers.reserve(threads);

    for (int t = 0; t < threads; ++t)
    {
        int start = t * chunk;
        int end = std::min(popsize, start + chunk);
        workers.emplace_back([&, start, end]() {
            for (int i = start; i < end; ++i)
            {
                if (specie[i].cached_cost < 1e17)
                {
                    costs[i] = specie[i].cached_cost;
                }
                else
                {
                    double c = Total_Cost_Individual(alpha, specie[i], total_stations, dist, ranking);
                    specie[i].cached_cost = c;
                    costs[i] = c;
                }
            }
        });
    }

    for (auto& w : workers) if (w.joinable()) w.join();

    return costs;
}

double RingCostActiveTour(int alpha,
    const std::vector<int>& active_ring,
    const std::vector<std::vector<double>>& dist)
{
    return RingCostOnly(alpha, active_ring, dist);
}
