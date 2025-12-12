#include "utils/Distance.h"
#include <cmath>
#include <algorithm>

std::vector<std::vector<double>>
Compute_Distances_2DVector(const std::vector<Node>& node_vector)
{
    int n = static_cast<int>(node_vector.size());
    std::vector<std::vector<double>> dist(n, std::vector<double>(n));

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            int dx = node_vector[i].x - node_vector[j].x;
            int dy = node_vector[i].y - node_vector[j].y;
            dist[i][j] = std::sqrt(dx * dx + dy * dy);
        }
    }
    return dist;
}

void Bubble_Sort(std::vector<int>& vector_int_to_sort,
    std::vector<double>& vector_double_to_sort)
{
    int taille = static_cast<int>(vector_int_to_sort.size());
    bool swapped = true;

    while (swapped)
    {
        swapped = false;
        for (int i = 0; i < taille - 1; i++)
        {
            if (vector_double_to_sort[i] > vector_double_to_sort[i + 1])
            {
                std::swap(vector_int_to_sort[i], vector_int_to_sort[i + 1]);
                std::swap(vector_double_to_sort[i], vector_double_to_sort[i + 1]);
                swapped = true;
            }
        }
    }
}

std::vector<std::vector<int>>
Distance_Ranking_2DVector(std::vector<std::vector<double>> distance_vector)
{
    int n = static_cast<int>(distance_vector.size());
    std::vector<std::vector<int>> ranking(n, std::vector<int>(n));

    std::vector<int> ids(n);
    std::vector<double> ds(n);

    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            ds[j] = distance_vector[i][j];
            ids[j] = j + 1; // id=1 at index 0
        }

        Bubble_Sort(ids, ds);

        for (int j = 0; j < n; ++j)
            ranking[i][j] = ids[j];
    }

    return ranking;
}

int Find_Nearest_Station(int city_id,
    const std::vector<bool>& mask,
    const std::vector<std::vector<int>>& ranking_vector)
{
    const std::vector<int>& row = ranking_vector[city_id - 1];

    for (int candidate_id : row)
    {
        if (mask[candidate_id - 1])
            return candidate_id;
    }
    return -1;
}

std::vector<int>
Assign_Stations(int number_of_stations,
    const std::vector<bool>& mask,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector)
{
    std::vector<int> assignment;

    for (int s = 1; s <= number_of_stations; ++s)
    {
        if (!mask[s - 1])
        {
            int nearest_station_id = Find_Nearest_Station(s, mask, ranking_vector);
            assignment.push_back(s);
            assignment.push_back(nearest_station_id);
        }
    }
    return assignment;
}
