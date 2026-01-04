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

    // Limit initial ring size to avoid huge rings on large datasets
    int max_initial_ring = std::min(20, std::max(3, total_stations / 5));

    for (int i = 0; i < species_number; ++i)
    {
        for (int j = 0; j < individual_number; ++j)
        {
            Individual ind;

            // Create a random ring with 3 to max_initial_ring active stations
            int ring_size = RandInt(3, std::min(max_initial_ring, total_stations));

            // Shuffle all station IDs but always include depot (1)
            std::vector<int> all_ids;
            for (int s = 2; s <= total_stations; ++s)
                all_ids.push_back(s);

            std::shuffle(all_ids.begin(), all_ids.end(), GLOBAL_RNG());

            ind.active_ring.clear();
            ind.active_ring.push_back(1); // ensure depot present at start

            // Fill up to ring_size (including depot)
            for (int k = 0; k < ring_size - 1 && k < (int)all_ids.size(); ++k)
                ind.active_ring.push_back(all_ids[k]);

            species[i].push_back(std::move(ind));
        }
    }

    return species;
}
