#pragma once
#include <vector>
#include "core/Individual.h"

// Insertion aléatoire (plus de calcul de coût greedy)
void Mutation_Add_Node(
    std::vector<int>& active_ring,
    int total_stations,
    const std::vector<std::vector<double>>& dist);

void Mutation_Remove_Node(
    std::vector<int>& active_ring,
    int min_ring_size = 3);

void Mutation_Swap_Simple(std::vector<int>& active_ring);
void Mutation_Inversion(std::vector<int>& active_ring);
void Mutation_Scramble(std::vector<int>& active_ring);

Individual Mutations(
    int add_percentage,
    int remove_percentage,
    int swap_percentage,
    int inversion_percentage,
    int scramble_percentage,
    const Individual& individual,
    int total_stations,
    const std::vector<std::vector<double>>& dist);