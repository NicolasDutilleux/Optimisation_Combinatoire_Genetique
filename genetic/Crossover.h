#pragma once
#include <vector>

// Slice crossover: take a slice from parent A, fill rest from parent B
std::vector<int> Slice_Crossover(
    const std::vector<int>& parentA,
    const std::vector<int>& parentB);
