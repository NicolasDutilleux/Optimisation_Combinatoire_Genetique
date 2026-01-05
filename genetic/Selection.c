// genetic/Selection.c - Pure C implementation
#include "Selection.h"

int Select_Best(const double* costs, int size)
{
    if (!costs || size <= 0) return -1;
    
    int best_idx = 0;
    double best_cost = costs[0];
    
    for (int i = 1; i < size; ++i) {
        if (costs[i] < best_cost) {
            best_cost = costs[i];
            best_idx = i;
        }
    }
    return best_idx;
}
