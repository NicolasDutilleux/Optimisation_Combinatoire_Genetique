// genetic/Crossover.c - Pure C implementation
#include "Crossover.h"
#include "utils\Random.h"
#include <stdlib.h>
#include <string.h>

int* Slice_Crossover(const int* parentA, int sizeA,
                     const int* parentB, int sizeB,
                     int* child_size, int max_capacity)
{
    int m = (sizeA < sizeB) ? sizeA : sizeB;
    if (m == 0) return NULL;
    
    int* child = (int*)malloc(max_capacity * sizeof(int));
    if (!child) return NULL;
    
    memset(child, 0, max_capacity * sizeof(int));
    
    int max_id = 0;
    for (int i = 0; i < sizeA; ++i) if (parentA[i] > max_id) max_id = parentA[i];
    for (int i = 0; i < sizeB; ++i) if (parentB[i] > max_id) max_id = parentB[i];
    
    if (max_id <= 0) {
        free(child);
        return NULL;
    }
    
    int cut1 = RandInt(0, m - 1);
    int cut2 = RandInt(0, m - 1);
    if (cut1 > cut2) {
        int tmp = cut1;
        cut1 = cut2;
        cut2 = tmp;
    }
    
    int* used = (int*)calloc(max_id + 1, sizeof(int));
    if (!used) {
        free(child);
        return NULL;
    }
    
    int child_idx = 0;
    for (int i = cut1; i <= cut2 && i < sizeA && child_idx < max_capacity; ++i) {
        child[child_idx++] = parentA[i];
        if (parentA[i] > 0 && parentA[i] <= max_id)
            used[parentA[i]] = 1;
    }
    
    for (int i = 0; i < sizeB && child_idx < max_capacity; ++i) {
        if (parentB[i] > 0 && parentB[i] <= max_id && !used[parentB[i]]) {
            child[child_idx++] = parentB[i];
            used[parentB[i]] = 1;
        }
    }
    
    *child_size = child_idx;
    free(used);
    return child;
}
