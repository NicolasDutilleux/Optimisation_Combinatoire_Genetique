#pragma once
#include <vector>

int Select_Best(const std::vector<double>& cost_vector_specie);
int Tournament_Select_Index(const std::vector<double>& costs, int k);
int PickParentIndex(const std::vector<int>& mating_pool, int mating_pool_size);
