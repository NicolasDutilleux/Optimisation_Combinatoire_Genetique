#include "evolution/EvolveSpecie.h"
#include "cost/Cost.h"
#include "genetic/Mutation.h"
#include "genetic/Crossover.h"
#include "genetic/Selection.h"
#include "local_search/TwoOpt.h"
#include "utils/Random.h"
#include <algorithm>
#include <thread>
#include <chrono>
#include <iostream>
#include <iomanip>

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
    // Chrono global
    auto t_start = std::chrono::high_resolution_clock::now();

    int popsize = static_cast<int>(specie.size());
    if (popsize <= 1) return;

    // --- PHASE 1: EVALUATION & SORT ---
    auto t_1 = std::chrono::high_resolution_clock::now();

    // 1) Evaluate all individuals
    std::vector<double> costs(popsize);
    for (int i = 0; i < popsize; ++i)
    {
        if (specie[i].cached_cost < 1e17) costs[i] = specie[i].cached_cost;
        else costs[i] = Total_Cost_Individual(alpha, specie[i], total_stations, dist, ranking);
        specie[i].cached_cost = costs[i];
    }

    // 2) Sort by cost
    std::vector<std::pair<double, int>> cost_index(popsize);
    for (int i = 0; i < popsize; ++i) cost_index[i] = { costs[i], i };
    std::sort(cost_index.begin(), cost_index.end());

    std::vector<int> sorted_indices(popsize);
    for (int i = 0; i < popsize; ++i) sorted_indices[i] = cost_index[i].second;

    auto t_eval_sort = std::chrono::high_resolution_clock::now();

    // --- PHASE 2: SELECTION ---
    std::vector<Individual> new_pop;
    new_pop.reserve(popsize);

    int elite = std::min(elitism_count, popsize);
    for (int e = 0; e < elite; ++e) {
        Individual elite_ind = specie[sorted_indices[e]];
        new_pop.push_back(std::move(elite_ind));
    }

    int need = popsize - static_cast<int>(new_pop.size());

    int mating_pool_size = std::max(2, static_cast<int>(mating_pool_fraction * popsize));
    std::vector<int> mating_pool(mating_pool_size);
    for (int i = 0; i < mating_pool_size; ++i) mating_pool[i] = sorted_indices[i];

    auto t_selection = std::chrono::high_resolution_clock::now();

    // --- PHASE 3: BREEDING (CROSSOVER + MUTATION) ---
    std::vector<Individual> children;
    children.reserve(need);
    while (static_cast<int>(children.size()) < need)
    {
        int i1 = mating_pool[RandInt(0, mating_pool_size - 1)];
        int i2 = mating_pool[RandInt(0, mating_pool_size - 1)];

        std::vector<int> child_ring = Slice_Crossover(specie[i1].active_ring, specie[i2].active_ring);
        Individual child;
        child.active_ring = std::move(child_ring);
        child.cached_cost = 1e18;

        child = Mutations(add_percentage, remove_percentage,
            swap_percentage, inversion_percentage, scramble_percentage,
            child, total_stations, dist);
        children.push_back(std::move(child));
    }

    auto t_breeding = std::chrono::high_resolution_clock::now();

    // --- PHASE 4: LOCAL SEARCH (2-OPT) ---
    unsigned int hw = std::thread::hardware_concurrency();
    if (hw == 0) hw = 4;
    int threads = std::min<unsigned int>(hw, children.size());
    int chunk = (static_cast<int>(children.size()) + threads - 1) / threads;

    std::vector<std::thread> workers;
    for (int t = 0; t < threads; ++t)
    {
        int start = t * chunk;
        int end = std::min((int)children.size(), start + chunk);
        workers.emplace_back([&, start, end]() {
            for (int i = start; i < end; ++i)
            {
                TwoOptImproveAlpha(children[i], alpha, dist, ranking);
                children[i].cached_cost = Total_Cost_Individual(alpha, children[i], total_stations, dist, ranking);
            }
            });
    }
    for (auto& w : workers) if (w.joinable()) w.join();

    for (auto& ch : children) new_pop.push_back(std::move(ch));
    specie.swap(new_pop);

    auto t_end = std::chrono::high_resolution_clock::now();

    // --- PRINTING (ALWAYS) ---
    // Suppression de la condition if (counter % 100) pour voir chaque gen
    auto d_eval = std::chrono::duration_cast<std::chrono::microseconds>(t_eval_sort - t_1).count();
    auto d_sel = std::chrono::duration_cast<std::chrono::microseconds>(t_selection - t_eval_sort).count();
    auto d_breed = std::chrono::duration_cast<std::chrono::microseconds>(t_breeding - t_selection).count();
    auto d_ls = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_breeding).count();
    auto d_total = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();

    std::cout << "[PROFILE] Total: " << std::setw(5) << d_total / 1000.0 << "ms | "
        << "Eval: " << std::setw(4) << d_eval / 1000.0 << "ms | "
        << "Sel: " << std::setw(4) << d_sel / 1000.0 << "ms | "
        << "Xover: " << std::setw(4) << d_breed / 1000.0 << "ms | "
        << "2-OPT: " << std::setw(5) << d_ls / 1000.0 << "ms ("
        << (int)(100.0 * d_ls / d_total) << "%)" << std::endl;
}