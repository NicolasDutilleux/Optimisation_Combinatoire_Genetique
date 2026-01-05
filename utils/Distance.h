#pragma once
#include "core\Node.h"

// Allocate 2D distance matrix
double** Compute_Distances_2DVector(const Node* nodes, int num_nodes);

// Allocate 2D ranking matrix  
int** Distance_Ranking_2DVector(double** dist, int rows, int cols);

// Free 2D arrays
void Free_2DArray_Double(double** arr, int rows);
void Free_2DArray_Int(int** arr, int rows);
