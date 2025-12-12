#pragma once
#include <vector>

#include "core/Individual.h"
#include "core/Node.h"

inline void Mutation_Swap_Simple(std::vector<int>& individual);

inline void Mutation_Inversion(std::vector<int>& individual);

inline void Mutation_Scramble(std::vector<int>& individual);

inline void Mutation_Insert_Swap(std::vector<int>& individual);

inline void Mutation_Deletion_Centroid(
    std::vector<int>& ids,
    std::vector<bool>& mask,
    const std::vector<Node>& nodes);

inline void Mutation_RandomMultiDeletion(
    const std::vector<int>& ids,
    std::vector<bool>& mask,
    int max_del,
    int min_active = 2);

inline void Mutation_Insertion_Heuristic(
    std::vector<int>& ids,
    std::vector<bool>& mask,
    int max_station_id,
    const std::vector<std::vector<double>>& dist);

Individual Mutations(
    int deletion_pourcentage,
    int multi_deletion_pourcentage,
    int swapping_pourcentage,
    int inversion_pourcentage,
    int scramble_pourcentage,
    int insertswap_pourcentage,
    const Individual& individual,
    int max_station_id,
    const std::vector<Node>& nodes,
    const std::vector<std::vector<double>>& dist);

