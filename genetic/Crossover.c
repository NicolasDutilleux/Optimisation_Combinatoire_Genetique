// genetic/Crossover.c - Optimized with stack allocation
//
// IMPORTANT: Le nœud 1 (dépôt) doit TOUJOURS être présent dans l'enfant !
//
#include "Crossover.h"
#include "utils\Random.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_STACK_SIZE 256

int* Slice_Crossover(const int* parentA, int sizeA,
                     const int* parentB, int sizeB,
                     int* child_size, int max_capacity)
{
    int m = (sizeA < sizeB) ? sizeA : sizeB;
    if (m == 0) return NULL;
    
    int* child = (int*)malloc(max_capacity * sizeof(int));
    if (!child) return NULL;
    
    // Find max ID
    int max_id = 0;
    for (int i = 0; i < sizeA; ++i) if (parentA[i] > max_id) max_id = parentA[i];
    for (int i = 0; i < sizeB; ++i) if (parentB[i] > max_id) max_id = parentB[i];
    
    if (max_id <= 0) {
        free(child);
        return NULL;
    }
    
    // Use stack for small arrays
    int stack_used[MAX_STACK_SIZE];
    int* used;
    int use_heap = (max_id + 1 > MAX_STACK_SIZE);
    
    if (use_heap) {
        used = (int*)calloc(max_id + 1, sizeof(int));
        if (!used) { free(child); return NULL; }
    } else {
        used = stack_used;
        memset(used, 0, (max_id + 1) * sizeof(int));
    }
    
    // Random cut points
    int cut1 = RandInt(0, m - 1);
    int cut2 = RandInt(0, m - 1);
    if (cut1 > cut2) { int tmp = cut1; cut1 = cut2; cut2 = tmp; }
    
    // Copy slice from parent A
    int child_idx = 0;
    for (int i = cut1; i <= cut2 && i < sizeA && child_idx < max_capacity; ++i) {
        child[child_idx++] = parentA[i];
        if (parentA[i] > 0 && parentA[i] <= max_id)
            used[parentA[i]] = 1;
    }
    
    // Fill from parent B
    for (int i = 0; i < sizeB && child_idx < max_capacity; ++i) {
        if (parentB[i] > 0 && parentB[i] <= max_id && !used[parentB[i]]) {
            child[child_idx++] = parentB[i];
            used[parentB[i]] = 1;
        }
    }
    
    // SÉCURITÉ : S'assurer que le dépôt (nœud 1) est dans l'enfant
    if (!used[1] && child_idx < max_capacity) {
        // Insérer le dépôt au début
        for (int i = child_idx; i > 0; i--) {
            child[i] = child[i - 1];
        }
        child[0] = 1;
        child_idx++;
    }
    
    *child_size = child_idx;
    if (use_heap) free(used);
    return child;
}
