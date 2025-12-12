#include "genetic/Selection.h"
#include "utils/Random.h"

int Select_Best(const std::vector<double>& costs)
{
    int best = 0;
    double best_cost = costs[0];

    for (int i = 1; i < (int)costs.size(); ++i)
    {
        if (costs[i] < best_cost)
        {
            best_cost = costs[i];
            best = i;
        }
    }
    return best;
}

int Tournament_Select_Index(const std::vector<double>& costs, int k)
{
    int n = static_cast<int>(costs.size());
    int best = RandInt(0, n - 1);

    for (int t = 1; t < k; ++t)
    {
        int cand = RandInt(0, n - 1);
        if (costs[cand] < costs[best])
            best = cand;
    }
    return best;
}

int PickParentIndex(const std::vector<int>& mating_pool, int mating_pool_size)
{
    int pos = RandInt(0, mating_pool_size - 1);
    return mating_pool[pos];
}
