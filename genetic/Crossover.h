#pragma once
#include <vector>

std::vector<int> Order_Crossover(const std::vector<int>& parentA,
    const std::vector<int>& parentB);

std::vector<bool> Mask_Crossover(const std::vector<bool>& m1,
    const std::vector<bool>& m2,
    int min_active = 2);
