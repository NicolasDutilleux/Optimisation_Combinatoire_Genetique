#include "generation/PopulationInit.h"
#include "utils/Random.h"

#include <algorithm>
#include <random>

std::vector<std::vector<Individual>> Random_Generation(
    const std::vector<Node>& node_vector,
    int species_number,
    int individual_number)
{
    std::vector<std::vector<Individual>> species(species_number);

    int station_number = node_vector.back().id; // keep your assumption

    std::vector<int> pool;
    pool.reserve(station_number);
    for (int x = 1; x <= station_number; ++x)
        pool.push_back(x);

    std::random_device rd;
    std::mt19937 rng(rd());

    for (int i = 0; i < species_number; ++i)
    {
        for (int j = 0; j < individual_number; ++j)
        {
            Individual ind;

            ind.ids = pool;
            std::shuffle(ind.ids.begin() + 1, ind.ids.end(), rng); // keep depot at front

            ind.mask.assign(station_number, false);
            ind.mask[0] = true;

            int active_count = RandInt(3, station_number);

            std::vector<int> candidate_indices;
            candidate_indices.reserve(station_number - 1);
            for (int k = 1; k < station_number; ++k)
                candidate_indices.push_back(k);

            std::shuffle(candidate_indices.begin(), candidate_indices.end(), rng);
            for (int k = 0; k < active_count - 1; ++k)
                ind.mask[candidate_indices[k]] = true;

            species[i].push_back(std::move(ind));
        }
    }

    return species;
}
