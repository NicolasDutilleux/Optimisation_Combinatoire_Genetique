#include <vector>
#include <cmath>
#include <random>
#include "Node.h"
#include <iostream>
#include <algorithm>

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
void Bubble_Sort(std::vector<int>& vector_int_to_sort, std::vector<double>& vector_double_to_sort)
{
    int taille = vector_int_to_sort.size();
    bool swapped = true;
    int swap_int = 0;
    double swap_double = 0;
    while (swapped)
    {
        swapped = false;
        for (int i = 0; i < taille - 1; i++)
        {
            if (vector_double_to_sort[i] > vector_double_to_sort[i + 1])
            {
                int swap_int = vector_int_to_sort[i];
                vector_int_to_sort[i] = vector_int_to_sort[i + 1];
                vector_int_to_sort[i + 1] = swap_int;
                double swap_double = vector_double_to_sort[i];
                vector_double_to_sort[i] = vector_double_to_sort[i + 1];
                vector_double_to_sort[i + 1] = swap_double;
                swapped = true;

            }
           //std::cout << vector_int_to_sort[0] << std::endl;
        }
       
    }


}
std::vector<std::vector<int>> Distance_Ranking_2DVector(std::vector<std::vector<double>> distance_vector)
{
	std::cout<<"INNN" << std::endl;
    int taille = distance_vector.size();
    std::vector<std::vector<int>> ranking_vector2D(taille, std::vector<int>(taille));
    std::vector<int> ranking_int_vector1D(taille);
    std::vector<double> ranking_double_vector1D(taille);
    int actual_best = 0;
    for (int i = 0; i < taille; i++)
    {

        for (int j = 0; j < taille; j++)
        {
            ranking_double_vector1D[j] = distance_vector[i][j];
            ranking_int_vector1D[j] = j + 1; // because the id 1 is at place "0"
        }
        Bubble_Sort(ranking_int_vector1D, ranking_double_vector1D);
        for (int j = 0; j < taille; j++)
        {
            ranking_vector2D[i][j] = ranking_int_vector1D[j];
        }
        std::cout << "Out " << i << std::endl;
    }
    std::cout << "Out" << std::endl;
	return ranking_vector2D;
}


std::vector<std::vector<std::vector<int>>> Random_Generation(const std::vector<Node>& node_vector,
    int species_number,
    int individual_number)
{
    std::vector<std::vector<std::vector<int>>> species(species_number);
    std::random_device rd;
    std::mt19937 rng(rd());

    int station_number = node_vector.back().id;

    // Construire le pool en commençant par 1
    std::vector<int> pool;
    pool.reserve(station_number);
    pool.push_back(1);
    for (int x = 2; x <= station_number; x++)
    {
        pool.push_back(x);
    }

    std::cout << "Size is " << pool.size() << std::endl;

    for (int i = 0; i < species_number; i++)
    {
        for (int j = 0; j < individual_number; j++)
        {
            // Copie du pool
            std::vector<int> individual = pool;

            // Mélanger tout sauf la première position (qui reste 1)
            std::shuffle(individual.begin() + 1, individual.end(), rng);

            species[i].push_back(individual);
        }
    }

    return species;
}

double Cost_station(int alpha, double distance)
{
    double cost = alpha * distance;

    return cost;
}

double Cost_out_ring(int alpha, double distance)
{
    double cost = (10 - alpha) * distance;

    return cost;
}
int Find_Nearest_Station(int city_id, std::vector<int>& individual, std::vector<std::vector<int>>& ranking_vector)
{
    int taille = individual.size();
    int nearest_station_id = -1;
    int taille_ranking = ranking_vector.size();
    for (int i = 0; i < ranking_vector.size(); i++)
    {
        int candidate_station_id = ranking_vector[city_id - 1][i]; // because id 1 is at place 0
        for (int j = 0; j < taille; j++)
        {
            if (individual[j] == candidate_station_id)
            {
                nearest_station_id = candidate_station_id;
                return nearest_station_id;
            }
        }
    }
    return -1; // Should never happen
}
double Total_Cost_Individual(int alpha, std::vector<int>& individual, std::vector<std::vector<double>>& dist, std::vector<std::vector<int>>& ranking_vector)
{
    int taille = individual.size();
	int number_of_stations = dist.size();
    double total_cost = 0;
    bool station_visited = false;
    for (int i = 0; i < taille; i++)
    {
        //std::cout << "Taille = " << taille << "i = " << i << std::endl;
        total_cost += Cost_station(alpha, dist[individual[i] - 1][individual[(i + 1) % taille] - 1]); // for performance : replace this by a substraction and comparison
    }
    
	// Check for unvisited stations
	for (int i = 1; i <= number_of_stations; i++)
    {
		station_visited = false;
		for (int j = 0; j < taille; j++)
        {
            if (individual[j] == i)
            {
                
				station_visited = true;
                break;
            }
        }


		// Trouver distance avec ville la plus proche
        if (!station_visited)
        {
			int nearest_station_id = Find_Nearest_Station(i, individual, ranking_vector);
            int distance_station = dist[i - 1 ][nearest_station_id - 1];

			total_cost += Cost_out_ring(alpha, distance_station); // Penalty for not visiting a station
        }
    }

    return total_cost;
}

std::vector<double> Total_Cost_Specie(int alpha, std::vector<std::vector<int>>& specie, std::vector<std::vector<double>>& dist, std::vector<std::vector<int>>& ranking_vector)
{
    int taille = specie.size();
    std::vector<double> cost_vector_specie;
    for (int i = 0; i < taille; i++)
    {
        cost_vector_specie.push_back(Total_Cost_Individual(alpha, specie[i], dist, ranking_vector));
    }
    
    return cost_vector_specie;
}

int Select_Best(std::vector<double> cost_vector_specie)
{
    int taille = cost_vector_specie.size();
    int best_index = 0;
    double actual_best = cost_vector_specie[0];
    for (int i = 0; i < taille; i++)
    {
        if (cost_vector_specie[i] < actual_best)
        {
            actual_best = cost_vector_specie[i];
            best_index = i;
        }

    }
    return best_index;
}
int Random(int minimum, int maximum)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(minimum, maximum);

    return distr(gen);
}

void Mutation_Swap(int swapping_pourcentage, std::vector<int>& individual)
{

    int taille = individual.size();
    int random_number = Random(1, 100);
    if (random_number > 100 - swapping_pourcentage)
    {
        int number_of_mutations = Random(1, ceil(taille / 2.0));
        int random_number_one = 0;
        int random_number_two = 0;
        for (int i = 0; i < number_of_mutations; i++)
        {

            random_number_one = Random(1, taille - 1); // 1 because we don't swap the depot
            random_number_two = Random(1, taille - 1);

            int swap = individual[random_number_one];
            individual[random_number_one] = individual[random_number_two];
            individual[random_number_two] = swap;
        }

    }

    return;
}

void Mutation_Deletion(int deletion_pourcentage, std::vector<int>& individual)
{
    // Si la mutation ne doit pas se produire, on renvoie simplement l'individu tel quel
    int roll = Random(1, 100);
    if (roll > 100 - deletion_pourcentage || individual.empty())
        return;


    // Choisir un index aléatoire
    int index = Random(1, individual.size() - 1);

    // Supprimer l'élément à cet index
    individual.erase(individual.begin() + index);


}
std::vector<int> Mutations(int deletion_pourcentage, int swapping_pourcentage, std::vector<int>& individual)
{
    std::vector<int> new_individual = individual;
    Mutation_Deletion(deletion_pourcentage, new_individual);
    Mutation_Swap(swapping_pourcentage, new_individual);
    return new_individual;

}

// Order Crossover (OX)
std::vector<int> Order_Crossover(const std::vector<int>& parentA, const std::vector<int>& parentB) {
    int taille = parentA.size();
    std::vector<int> child(taille, -1);

    // Choisir deux points de coupure
    int cut1 = Random(1, taille - 2); // on évite la première position (dépôt)
    int cut2 = Random(cut1 + 1, taille - 1);

    // Copier le segment du parent A
    for (int i = cut1; i <= cut2; i++) {
        child[i] = parentA[i];
    }

    // Compléter avec les éléments du parent B
    int currentIndex = (cut2 + 1) % taille;
    for (int i = 0; i < taille; i++) {
        int candidate = parentB[(cut2 + 1 + i) % taille];

        // Vérifier si déjà présent
        if (std::find(child.begin(), child.end(), candidate) == child.end()) {
            child[currentIndex] = candidate;
            currentIndex = (currentIndex + 1) % taille;
        }
    }

    return child;
}
