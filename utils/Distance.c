// utils/Distance.c - Pure C implementation
#include "Distance.h"
#include <stdlib.h>
#include <math.h>

typedef struct {
    double value;
    int index;
} DistPair;

static int cmp_dist(const void* a, const void* b)
{
    double diff = ((DistPair*)a)->value - ((DistPair*)b)->value;
    if (diff < 0) return -1;
    if (diff > 0) return 1;
    return ((DistPair*)a)->index - ((DistPair*)b)->index;
}

double** Compute_Distances_2DVector(const Node* nodes, int num_nodes)
{
    double** dist = (double**)malloc(num_nodes * sizeof(double*));
    if (!dist) return NULL;
    
    for (int i = 0; i < num_nodes; ++i) {
        dist[i] = (double*)malloc(num_nodes * sizeof(double));
        if (!dist[i]) {
            Free_2DArray_Double(dist, i);
            return NULL;
        }
    }
    
    for (int i = 0; i < num_nodes; ++i) {
        dist[i][i] = 0.0;
        for (int j = i + 1; j < num_nodes; ++j) {
            double dx = nodes[i].x - nodes[j].x;
            double dy = nodes[i].y - nodes[j].y;
            double d = sqrt(dx * dx + dy * dy);
            dist[i][j] = d;
            dist[j][i] = d;
        }
    }
    
    return dist;
}

int** Distance_Ranking_2DVector(double** dist, int rows, int cols)
{
    int** ranking = (int**)malloc(rows * sizeof(int*));
    if (!ranking) return NULL;
    
    for (int i = 0; i < rows; ++i) {
        ranking[i] = (int*)malloc(cols * sizeof(int));
        if (!ranking[i]) {
            Free_2DArray_Int(ranking, i);
            return NULL;
        }
    }
    
    for (int i = 0; i < rows; ++i) {
        DistPair* pairs = (DistPair*)malloc(cols * sizeof(DistPair));
        if (!pairs) {
            Free_2DArray_Int(ranking, rows);
            return NULL;
        }
        
        for (int j = 0; j < cols; ++j) {
            pairs[j].value = dist[i][j];
            pairs[j].index = j + 1;
        }
        
        qsort(pairs, cols, sizeof(DistPair), cmp_dist);
        
        for (int j = 0; j < cols; ++j) {
            ranking[i][j] = pairs[j].index;
        }
        
        free(pairs);
    }
    
    return ranking;
}

void Free_2DArray_Double(double** arr, int rows)
{
    if (!arr) return;
    for (int i = 0; i < rows; ++i) {
        if (arr[i]) free(arr[i]);
    }
    free(arr);
}

void Free_2DArray_Int(int** arr, int rows)
{
    if (!arr) return;
    for (int i = 0; i < rows; ++i) {
        if (arr[i]) free(arr[i]);
    }
    free(arr);
}
