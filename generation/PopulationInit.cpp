#include "generation/PopulationInit.h"
#include "utils/Random.h"
#include <algorithm>

std::vector<std::vector<Individual>> Random_Generation(
    const std::vector<Node>& node_vector,
    int species_number,
    int individual_number)
{
    std::vector<std::vector<Individual>> species(species_number);

    int total_stations = static_cast<int>(node_vector.size());

    for (int i = 0; i < species_number; ++i)
    {
        for (int j = 0; j < individual_number; ++j)
        {
            Individual ind;

            // Create a random ring with 3 to total_stations active stations
            int ring_size = RandInt(3, total_stations);

            // Shuffle all station IDs
            std::vector<int> all_ids;
            for (int s = 1; s <= total_stations; ++s)
                all_ids.push_back(s);

            std::shuffle(all_ids.begin(), all_ids.end(), GLOBAL_RNG());

            // Keep the first ring_size stations
            ind.active_ring.assign(all_ids.begin(), all_ids.begin() + ring_size);

            species[i].push_back(std::move(ind));
        }
    }

    return species;
}
