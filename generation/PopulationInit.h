#pragma once
#include <vector>
#include "core/Node.h"
#include "core/Individual.h"

std::vector<std::vector<Individual>> Random_Generation(
    const std::vector<Node>& node_vector,
    int species_number,
    int individual_number);
