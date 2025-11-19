#include <vector>
#include <cmath>
#include <random>
#include "Node.h"
#include <iostream>

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
std::vector<std::vector<int>> Distance_Ranking_2DVector(std::vector<std::vector<double>> distance_vector)
{
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
    }
}

void Bubble_Sort(std::vector<int>& vector_int_to_sort, std::vector<double>& vector_double_to_sort)
{
    int taille = vector_int_to_sort.size();
    bool swapped = false;
    int swap_int = 0;
    double swap_double = 0;
    while (swapped = true)
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
        }
    }


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
    std::cout << "Size is " << pool.size() << std::endl;
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
double Cost_station(int alpha, double distance)
{
    double cost = alpha * distance;

    return cost;
}

double Total_Cost_Individual(int alpha, std::vector<int>& individual, std::vector<std::vector<double>>& dist)
{
    int taille = individual.size();
    double total_cost = 0;
    for (int i = 0; i < taille; i++)
    {
        //std::cout << "Taille = " << taille << "i = " << i << std::endl;
        total_cost += Cost_station(alpha, dist[individual[i] - 1][individual[(i + 1) % taille] - 1]); // for performance : replace this by a substraction and comparison
    }

    return total_cost;
}

std::vector<double> Total_Cost_Specie(int alpha, std::vector<std::vector<int>>& specie, std::vector<std::vector<double>>& dist)
{
    int taille = specie.size();
    std::vector<double> cost_vector_specie;
    for (int i = 0; i < taille; i++)
    {
        cost_vector_specie.push_back(Total_Cost_Individual(alpha, specie[i], dist));
    }
    return cost_vector_specie;
}

int Select_Best(std::vector<double> cost_vector_specie)
{
    int taille = cost_vector_specie.size();
    int best_index = 0;
    double actual_best = 0;
    for (int i = 0; i < taille; i++)
    {
        if (cost_vector_specie[i] < actual_best || actual_best == 0)
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

std::vector<int> Mutation_Swap(int swapping_pourcentage, std::vector<int>& individual)
{

    int taille = individual.size();
    int random_number = Random(1, 100);
    if (random_number > 100 - swapping_pourcentage)
    {
        int number_of_mutations = Random(1, ceil(taille / 3.0));
        int random_number_one = 0;
        int random_number_two = 0;
        for (int i = 0; i < number_of_mutations; i++)
        {
            //std::cout << number_of_mutations << " Mutations" << std::endl;
            //std::cout << "kakouuuu  " << taille - 1 << std::endl;
            random_number_one = Random(0, taille - 1);
            random_number_two = Random(0, taille - 1);

            //std::cout << "kakouuuu  " << taille << " rn1 " << random_number_one << " rn2 " << random_number_two << std::endl;
            //new_individual[random_number_one] = individual[random_number_two];
            //new_individual[random_number_two] = individual[random_number_one];
            int swap = individual[random_number_one];
            individual[random_number_one] = individual[random_number_two];
            individual[random_number_two] = swap;
        }

    }

    return individual;
}

std::vector<int> Mutation_Deletion(int deletion_pourcentage, std::vector<int>& individual)
{
    // Si la mutation ne doit pas se produire, on renvoie simplement l'individu tel quel
    int roll = Random(1, 100);
    if (roll > 100 - deletion_pourcentage || individual.empty())
        return individual;


    // Choisir un index aléatoire
    int index = Random(0, individual.size() - 1);

    // Supprimer l'élément à cet index
    individual.erase(individual.begin() + index);

    return individual;
}
std::vector<int> Mutations(int deletion_pourcentage, int swapping_pourcentage, std::vector<int>& individual)
{
    std::vector<int> new_individual = individual;
    Mutation_Deletion(deletion_pourcentage, new_individual);
    Mutation_Swap(swapping_pourcentage, new_individual);
    return individual;

}
void Print_Double_Vector(std::vector<double> cost_vector)
{
    std::cout << cost_vector.size() << std::endl;
    for (int i = 0; i < cost_vector.size(); i++)
    {
        std::cout << "Total cost " << i << " = " << cost_vector[i] << "       " << i << "  \n";
    }
}
void Print_Individual(std::vector<int>& individual)
{
    int taille = individual.size();
    for (int i = 0; i < taille; i++)
    {
        std::cout << individual[i] << " ";
    }
    std::cout << std::endl;
}
void Print_Specie(std::vector<std::vector<int>> specie)
{
    int taille = specie.size();

    for (int i = 0; i < taille; i++)
    {
        std::cout << "Individual Number [" << i << "] " << std::endl;
        Print_Individual(specie[i]);
    }


}
