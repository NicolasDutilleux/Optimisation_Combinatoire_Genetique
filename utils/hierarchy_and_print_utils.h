#pragma once

#include "core\Node.h"
#include "core\Individual.h"

#ifdef __cplusplus
extern "C" {
#endif

// C helpers used by C code
int* BuildAssignmentPairs(const int* active_ring, int ring_size, const int** ranking, int total_stations, int* out_len);

void PlotIndividualSVG_C(const int* active_ring, int ring_size, const Node* node_vector, int num_nodes,
    int generation, const int* assignment_pairs, int assignment_len);

#ifdef __cplusplus
}
#endif