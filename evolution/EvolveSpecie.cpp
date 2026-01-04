#include "evolution/EvolveSpecie.h"
#include "cost/Cost.h"
#include "genetic/Mutation.h"
#include "genetic/Crossover.h"
#include "genetic/Selection.h"
#include "local_search/TwoOpt.h"
#include "utils/Random.h"
#include <algorithm>
#include <iostream>
#include <chrono>

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

    // 1) Evaluate all individuals
    std::vector<double> costs = Total_Cost_Specie(alpha, specie, total_stations, dist, ranking);

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
        new_pop.push_back(specie[sorted_indices[e]]);

    // 4) Build mating pool
    int mating_pool_size = std::max(2, static_cast<int>(mating_pool_fraction * popsize));
    if (mating_pool_size > popsize) mating_pool_size = popsize;

    std::vector<int> mating_pool(mating_pool_size);
    for (int i = 0; i < mating_pool_size; ++i)
        mating_pool[i] = sorted_indices[i];

    // 5) Fill rest of population via crossover & mutation
    while (static_cast<int>(new_pop.size()) < popsize)
    {
        int i1 = mating_pool[RandInt(0, mating_pool_size - 1)];
        int i2 = mating_pool[RandInt(0, mating_pool_size - 1)];

        const Individual& p1 = specie[i1];
        const Individual& p2 = specie[i2];

        std::vector<int> child_ring = Slice_Crossover(p1.active_ring, p2.active_ring);

        Individual child;
        child.active_ring = std::move(child_ring);

        bool force_mutation = (child.active_ring == p1.active_ring || 
                               child.active_ring == p2.active_ring);

        if (force_mutation || RandDouble() < mutation_rate)
        {
            child = Mutations(add_percentage, remove_percentage,
                              swap_percentage, inversion_percentage, scramble_percentage,
                              child, total_stations, dist);
        }

        // OPTIMIZATION: Apply 2-opt to ALL children (not just 20%)
        // because it's now fast with limited iterations
        TwoOptImproveAlpha(child, alpha, dist, ranking);

        new_pop.push_back(std::move(child));
    }

    specie.swap(new_pop);
}

