#include <vector>
#include <cmath>
#include <random>
#include "Node.h"

std::vector<std::vector<double>> Compute_Distances_2DVector(const std::vector<Node>& node_vector)
{
    int n = node_vector.size();
    std::vector<std::vector<double>> dist(n, std::vector<double>(n));

    // CAN OPITMISE BY NOT CALCULATING THE REDUNDANCY

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            int dx = node_vector[i].x - node_vector[j].x;
            int dy = node_vector[i].y - node_vector[j].y;
            dist[i][j] = sqrt(dx * dx + dy * dy);
        }
    }

    // Compute the distances between each point for futur maths (sqrt((x-x)^2 + (y-y)^2))

    return dist;
}

std::vector<std::vector<std::vector<int>>> Random_Generation(const std::vector<Node>& node_vector, int species_number, int individual_number)
{
    std::vector<std::vector<std::vector<int>>> species(species_number);
    std::random_device rd;
    std::mt19937 rng(rd());

    int station_number = node_vector[node_vector.size() - 1].id;
    std::vector<int> pool;
    for (int x = 1; x <= station_number; x++)
    {
        pool.push_back(x);
    }

    for (int i = 0; i < species_number; i++)
    {
        for (int j = 0; j < individual_number; j++)
        {

            // Start from the full pool
            std::vector<int> individual = pool;

            // Shuffle to get a random permutation
            std::shuffle(individual.begin(), individual.end(), rng);

            species[i].push_back(individual);
        }
    }

    return species;

}

