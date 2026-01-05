// evolution/EvolveSpecie.c - Pure C implementation (safe, with logging and validation)
#include "EvolveSpecie.h"
#include "cost\Cost.h"
#include "genetic\Mutation.h"
#include "genetic\Crossover.h"
#include "genetic\Selection.h"
#include "local_search\TwoOpt.h"
#include "utils\Random.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    double cost;
    int index;
} CostIndex;

static int cmp_cost(const void* a, const void* b)
{
    double diff = ((CostIndex*)a)->cost - ((CostIndex*)b)->cost;
    if (diff < 0) return -1;
    if (diff > 0) return 1;
    return 0;
}

static int validate_individual(const Individual* ind, int total_stations)
{
    if (!ind) return 0;
    if (!ind->active_ring) return 0;
    if (ind->ring_size < 0 || ind->ring_size > ind->ring_capacity) return 0;
    for (int i = 0; i < ind->ring_size; ++i) {
        int id = ind->active_ring[i];
        if (id <= 0 || id > total_stations) return 0;
    }
    return 1;
}

void EvolveSpecie(Individual* specie, int specie_size,
                  const double** dist, const int** ranking,
                  const Node* nodes, int num_nodes,
                  int alpha, int total_stations,
                  double mutation_rate, int elitism_count,
                  int add_percentage, int remove_percentage,
                  int swap_percentage, int inversion_percentage,
                  int scramble_percentage, double mating_pool_fraction)
{
    if (!specie || specie_size <= 0) return;

    //fprintf(stderr, "[EVOLVE] start specie=%p size=%d\n", (void*)specie, specie_size);
    fflush(stderr);

    // 1) Evaluate all individuals
    double* costs = (double*)malloc(specie_size * sizeof(double));
    if (!costs) return;

    for (int i = 0; i < specie_size; ++i) {
        if (!specie[i].active_ring) {
            Individual_Init(&specie[i], 50);
            //fprintf(stderr, "[EVOLVE] init missing individual %d\n", i);
            fflush(stderr);
        }
        //fprintf(stderr, "[EVOLVE] indiv %d ring_size=%d cap=%d ptr=%p\n", i, specie[i].ring_size, specie[i].ring_capacity, (void*)specie[i].active_ring);
        fflush(stderr);
        if (!validate_individual(&specie[i], total_stations)) {
            //fprintf(stderr, "[EVOLVE] Warning: invalid individual %d detected, marking cost INF\n", i);
            fflush(stderr);
            costs[i] = 1e18;
            specie[i].cached_cost = 1e18;
            continue;
        }
        if (specie[i].cached_cost < 1e17) {
            costs[i] = specie[i].cached_cost;
        } else {
            costs[i] = Total_Cost_Individual(alpha, &specie[i], total_stations, dist, ranking);
            specie[i].cached_cost = costs[i];
        }
        //fprintf(stderr, "[EVOLVE] indiv %d cost=%.2f\n", i, costs[i]); fflush(stderr);
    }

    // 2) Sort by cost
    CostIndex* cost_index = (CostIndex*)malloc(specie_size * sizeof(CostIndex));
    if (!cost_index) { free(costs); return; }
    for (int i = 0; i < specie_size; ++i) { cost_index[i].cost = costs[i]; cost_index[i].index = i; }
    qsort(cost_index, specie_size, sizeof(CostIndex), cmp_cost);

    int* sorted_indices = (int*)malloc(specie_size * sizeof(int));
    if (!sorted_indices) { free(costs); free(cost_index); return; }
    for (int i = 0; i < specie_size; ++i) sorted_indices[i] = cost_index[i].index;

    // 3) Create new population (in-place initialization)
    Individual* new_pop = (Individual*)malloc(specie_size * sizeof(Individual));
    if (!new_pop) { free(costs); free(cost_index); free(sorted_indices); return; }
    for (int i = 0; i < specie_size; ++i) Individual_Init(&new_pop[i], 50);

    int elite = (elitism_count < specie_size) ? elitism_count : specie_size;
    for (int e = 0; e < elite; ++e) {
        Individual_Copy(&new_pop[e], &specie[sorted_indices[e]]);
        new_pop[e].cached_cost = costs[sorted_indices[e]];
    }

    int need = specie_size - elite;
    if (need <= 0) {
        for (int i = 0; i < specie_size; ++i) {
            Individual_Free(&specie[i]);
            specie[i] = new_pop[i];
        }
        free(costs); free(cost_index); free(sorted_indices); free(new_pop);
        return;
    }

    // 4) Build mating pool
    int mating_pool_size = (int)(mating_pool_fraction * specie_size);
    if (mating_pool_size < 2) mating_pool_size = 2;
    if (mating_pool_size > specie_size) mating_pool_size = specie_size;
    int* mating_pool = (int*)malloc(mating_pool_size * sizeof(int));
    if (!mating_pool) {
        free(costs); free(cost_index); free(sorted_indices);
        for (int i = 0; i < specie_size; ++i) Individual_Free(&new_pop[i]); free(new_pop);
        return;
    }
    for (int i = 0; i < mating_pool_size; ++i) mating_pool[i] = sorted_indices[i];

    // 5) Generate children
    Individual* children = (Individual*)malloc(need * sizeof(Individual));
    if (!children) { free(costs); free(cost_index); free(sorted_indices); free(mating_pool); for (int i=0;i<specie_size;++i) Individual_Free(&new_pop[i]); free(new_pop); return; }
    for (int i = 0; i < need; ++i) Individual_Init(&children[i], 50);

    for (int i = 0; i < need; ++i) {
        int i1 = mating_pool[RandInt(0, mating_pool_size - 1)];
        int i2 = mating_pool[RandInt(0, mating_pool_size - 1)];
        Individual* p1 = &specie[i1];
        Individual* p2 = &specie[i2];

        int child_size = 0;
        int* child_ring = Slice_Crossover(p1->active_ring, p1->ring_size, p2->active_ring, p2->ring_size, &child_size, 50);
        if (child_ring) {
            if (child_size > children[i].ring_capacity) {
                int* tmp = (int*)realloc(children[i].active_ring, child_size * sizeof(int));
                if (tmp) { children[i].active_ring = tmp; children[i].ring_capacity = child_size; }
            }
            memcpy(children[i].active_ring, child_ring, child_size * sizeof(int));
            children[i].ring_size = child_size;
            children[i].cached_cost = 1e18;
            free(child_ring);
        }

        int force_mutation = 0;
        if (children[i].ring_size == p1->ring_size && children[i].ring_size > 0) {
            if (memcmp(children[i].active_ring, p1->active_ring, children[i].ring_size * sizeof(int)) == 0) force_mutation = 1;
        }
        if (!force_mutation && children[i].ring_size == p2->ring_size && children[i].ring_size > 0) {
            if (memcmp(children[i].active_ring, p2->active_ring, children[i].ring_size * sizeof(int)) == 0) force_mutation = 1;
        }

        if (force_mutation || RandDouble() < mutation_rate) {
            Individual temp; Individual_Init(&temp, 50);
            Mutations(&temp, add_percentage, remove_percentage, swap_percentage, inversion_percentage, scramble_percentage, &children[i], total_stations, dist);
            Individual_Copy(&children[i], &temp);
            Individual_Free(&temp);
        }
    }

    // 6) Improve children
    for (int i = 0; i < need; ++i) {
        TwoOptImproveAlpha(&children[i], alpha, dist, ranking);
        double c = Total_Cost_Individual(alpha, &children[i], total_stations, dist, ranking);
        children[i].cached_cost = c;
    }
    //fprintf(stderr, "[EVOLVE] children improved\n"); fflush(stderr);

    // 7) Append children to new population
    for (int i = 0; i < need; ++i) {
        Individual_Copy(&new_pop[elite + i], &children[i]);
        Individual_Free(&children[i]);
    }

    // Replace old species
    for (int i = 0; i < specie_size; ++i) {
        Individual_Free(&specie[i]);
        specie[i] = new_pop[i];
    }

    // cleanup
    free(costs); free(cost_index); free(sorted_indices); free(mating_pool); free(new_pop); free(children);
    //fprintf(stderr, "[EVOLVE] exit specie=%p\n", (void*)specie); fflush(stderr);
}

// Add sanity function implementation
void EvolveSpecie_Sanity(Individual* specie)
{
    //fprintf(stderr, "[EVOLVE_SANITY] called for %p\n", (void*)specie);
    fflush(stderr);
}
