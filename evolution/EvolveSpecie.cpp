#include "evolution/EvolveSpecie.h"

#include "cost/Cost.h"
#include "genetic/Selection.h"
#include "genetic/Crossover.h"
#include "genetic/Mutation.h"

#include "local_search/TwoOpt.h"
#include "local_search/MaskLocalSearch.h"

#include "utils/Random.h"

#include <algorithm>
#include <chrono>
#include <iostream>


using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;

void EvolveSpecie(
    std::vector<Individual>& specie,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking,
    const std::vector<Node>& nodes,
    int alpha,
    double mutation_rate,
    int max_station_id,
    int elitism_count,
    int DEL_PCT_RDM,
    int DEL_PCT_CENT,
    int SWAP_PCT,
    int INV_PCT,
    int SCR_PCT,
    int INSERTSWAP_PCT,
    double max_mating_pool_size,
    bool time_bool
)
{
    int popsize = static_cast<int>(specie.size());
    if (popsize <= 1) return;

    TimePoint CP_1 = Clock::now();
    // --- 1) Compute costs for the whole specie ---
    std::vector<double> costs = Total_Cost_Specie(alpha,
        specie,
        dist,
        ranking);
    TimePoint CP_2 = Clock::now(); // Capture the end time
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(CP_2 - CP_1);
    // Print the result
    if (time_bool) { std::cout << "Code block executed [Total_Cost_Specie] " << duration_ms.count() << " ms\n"; }
    // --- 2) Sort indices by increasing cost (best first) ---
    std::vector<std::pair<double, int>> cost_index(popsize);

    for (int i = 0; i < popsize; i++)
        cost_index[i] = { costs[i], i };  // (cost, index)

    std::sort(cost_index.begin(), cost_index.end());  // sorts by cost automatically

    // Extract sorted indices
    std::vector<int> indices(popsize);
    for (int i = 0; i < popsize; i++)
        indices[i] = cost_index[i].second;


    // --- 3) Build new population container ---
    std::vector<Individual> new_pop;
    new_pop.reserve(popsize);


    // (a) Elitism: copy the best 'elitism_count' individuals as-is
    int elite = std::min(elitism_count, popsize);
    for (int e = 0; e < elite; ++e)
    {
        new_pop.push_back(specie[indices[e]]);
    }

    // (b) Build mating pool = the best 'mating_pool_size' individuals
    int mating_pool_size = std::max(2, static_cast<int>(max_mating_pool_size * popsize));
    if (mating_pool_size > popsize) mating_pool_size = popsize;

    std::vector<int> mating_pool(mating_pool_size);
    for (int i = 0; i < mating_pool_size; ++i)
    {
        mating_pool[i] = indices[i]; // indices of best individuals
    }


    TimePoint CP_3 = Clock::now(); // Capture the end time
    duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(CP_3 - CP_2);
    // Print the result
    if (time_bool) { std::cout << "Code block executed [Before_Mating] " << duration_ms.count() << " ms\n"; }

    // --- 5) Fill the rest of the population ---
    while (static_cast<int>(new_pop.size()) < popsize)
    {
        // Parents chosen ONLY among the best 'mating_pool_size' individuals
        int i1 = PickParentIndex(mating_pool, mating_pool_size);
        int i2 = PickParentIndex(mating_pool, mating_pool_size);

        const Individual& p1 = specie[i1];
        const Individual& p2 = specie[i2];

        // Crossover sur la permutation (ids)
        std::vector<int> child_ids = Order_Crossover(p1.ids, p2.ids);

        // Crossover génétique sur le mask
        std::vector<bool> child_mask =
            Mask_Crossover(p1.mask, p2.mask, /*min_active*/ 2);

        Individual child;
        child.ids = std::move(child_ids); // basically change the pointer at the place of copying. Apparently faster that's waht tehy said.
        child.mask = std::move(child_mask);

        // Mutation decision:
        bool force_mutation = (child.ids == p1.ids) || (child.ids == p2.ids);

        if (force_mutation || RandDouble() < mutation_rate)
        {
            child = Mutations(DEL_PCT_RDM,
                DEL_PCT_CENT,
                SWAP_PCT,
                INV_PCT,
                SCR_PCT,
                INSERTSWAP_PCT,
                child,
                max_station_id,
                nodes,
                dist);
        }

        // If you want purely genetic GA, keep 2-opt commented out:
        // TwoOptImproveAlpha(child, alpha, dist, ranking);
        TwoOptImproveAlpha(child, alpha, dist, ranking);

        new_pop.push_back(std::move(child));
    }

    TimePoint CP_4 = Clock::now(); // Capture the end time
    duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(CP_4 - CP_3);
    // Print the result
    if (time_bool) { std::cout << "Code block executed [After_Mating] " << duration_ms.count() << " ms\n"; }

    // --- 5.5) Optional: local search on mask for the best of this specie ---
    int best_idx = 0;
    double best_cost = 1e18;
    for (int i = 0; i < (int)new_pop.size(); ++i)
    {
        double c = Total_Cost_Individual(alpha,
            new_pop[i].ids,
            new_pop[i].mask,
            dist,
            ranking);
        if (c < best_cost)
        {
            best_cost = c;
            best_idx = i;
        }
    }
    TimePoint CP_5 = Clock::now(); // Capture the end time
    duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(CP_5 - CP_4);
    // Print the result
    if (time_bool) { std::cout << "Code block executed [After_Local_Search] " << duration_ms.count() << " ms\n"; }
    // Hill climber sur le mask du meilleur de l'espèce

    ImproveMaskLocal(new_pop[best_idx],
        alpha,
        max_station_id,
        dist,
        ranking);
    // --- 6) Replace old specie by the new one ---
    specie.swap(new_pop);


    TimePoint CP_6 = Clock::now(); // Capture the end time
    duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(CP_6 - CP_5);
    // Print the result
    if (time_bool) { std::cout << "Code block executed [After_Improve_Local_Mask] " << duration_ms.count() << " ms\n"; }

}
  

