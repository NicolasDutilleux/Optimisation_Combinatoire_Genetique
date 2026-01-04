#include "genetic/Selection.h"

int Select_Best(const std::vector<double>& costs)
{
    int best_idx = 0;
    double best_cost = costs[0];

    for (int i = 1; i < static_cast<int>(costs.size()); ++i)
    {
        if (costs[i] < best_cost)
        {
            best_cost = costs[i];
            best_idx = i;
        }
    }
    return best_idx;
}
