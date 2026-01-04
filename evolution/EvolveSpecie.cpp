#include "evolution/EvolveSpecie.h"
#include "cost/Cost.h"
#include "genetic/Mutation.h"
#include "genetic/Crossover.h"
#include "genetic/Selection.h"
#include "local_search/TwoOpt.h"
#include "utils/Random.h"
#include <algorithm>
#include <thread>

void EvolveSpecie(
    std::vector<Individual>& specie,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking,
    const std::vector<Node>& nodes,
    int alpha,
    int total_stations,
    double mutation_rate,
    int elitism_count,
    int add_percentage,
    int remove_percentage,
    int swap_percentage,
    int inversion_percentage,
    int scramble_percentage,
    double mating_pool_fraction)
{
    int popsize = static_cast<int>(specie.size());
    if (popsize <= 1) return;

    // 1) Evaluate all individuals (use cached_cost when available)
    std::vector<double> costs(popsize);
    for (int i = 0; i < popsize; ++i)
    {
        if (specie[i].cached_cost < 1e17) costs[i] = specie[i].cached_cost;
        else costs[i] = Total_Cost_Individual(alpha, specie[i], total_stations, dist, ranking);
        // store cached value
        specie[i].cached_cost = costs[i];
    }

    // 2) Sort by cost
    std::vector<std::pair<double, int>> cost_index(popsize);
    for (int i = 0; i < popsize; ++i)
        cost_index[i] = { costs[i], i };

    std::sort(cost_index.begin(), cost_index.end());

    std::vector<int> sorted_indices(popsize);
    for (int i = 0; i < popsize; ++i)
        sorted_indices[i] = cost_index[i].second;

    // 3) Create new population with elitism
    std::vector<Individual> new_pop;
    new_pop.reserve(popsize);

    int elite = std::min(elitism_count, popsize);
    for (int e = 0; e < elite; ++e)
    {
        Individual elite_ind = specie[sorted_indices[e]];
        elite_ind.cached_cost = costs[sorted_indices[e]];
        new_pop.push_back(std::move(elite_ind));
    }

    int need = popsize - static_cast<int>(new_pop.size());
    if (need <= 0)
    {
        specie.swap(new_pop);
        return;
    }

    // 4) Build mating pool
    int mating_pool_size = std::max(2, static_cast<int>(mating_pool_fraction * popsize));
    if (mating_pool_size > popsize) mating_pool_size = popsize;

    std::vector<int> mating_pool(mating_pool_size);
    for (int i = 0; i < mating_pool_size; ++i)
        mating_pool[i] = sorted_indices[i];

    // 5) Generate children (lightweight: no 2-opt, no cost yet)
    std::vector<Individual> children;
    children.reserve(need);
    while (static_cast<int>(children.size()) < need)
    {
        int i1 = mating_pool[RandInt(0, mating_pool_size - 1)];
        int i2 = mating_pool[RandInt(0, mating_pool_size - 1)];

        const Individual& p1 = specie[i1];
        const Individual& p2 = specie[i2];

        std::vector<int> child_ring = Slice_Crossover(p1.active_ring, p2.active_ring);

        Individual child;
        child.active_ring = std::move(child_ring);
        child.cached_cost = 1e18; // mark unevaluated

        bool force_mutation = (child.active_ring == p1.active_ring || 
                               child.active_ring == p2.active_ring);

        if (force_mutation || RandDouble() < mutation_rate)
        {
            child = Mutations(add_percentage, remove_percentage,
                              swap_percentage, inversion_percentage, scramble_percentage,
                              child, total_stations, dist);
            child.cached_cost = 1e18;
        }

        children.push_back(std::move(child));
    }

    // 6) Improve children in parallel (TwoOpt + cost eval)
    unsigned int hw = std::thread::hardware_concurrency();
    if (hw == 0) hw = 4;
    int threads = std::min<unsigned int>(hw, children.size());
    int chunk = (static_cast<int>(children.size()) + threads - 1) / threads;

    std::vector<std::thread> workers;
    workers.reserve(threads);

    for (int t = 0; t < threads; ++t)
    {
        int start = t * chunk;
        int end = std::min((int)children.size(), start + chunk);
        workers.emplace_back([&, start, end]() {
            for (int i = start; i < end; ++i)
            {
                // Local search
                TwoOptImproveAlpha(children[i], alpha, dist, ranking);
                // Evaluate cost
                double c = Total_Cost_Individual(alpha, children[i], total_stations, dist, ranking);
                children[i].cached_cost = c;
            }
        });
    }
    for (auto& w : workers) if (w.joinable()) w.join();

    // 7) Append children to new population
    for (auto& ch : children)
        new_pop.push_back(std::move(ch));

    specie.swap(new_pop);
}

