#include "logic_function.h"
#include "hierarchy_and_print_utils.h"


int main()
{
    // Basic Tests
    current_folder_path();

    std::cout << "Hello World!\n\n";

    // LOAD DATA FROM NOTEBOOK

    std::vector<Node> node_vector = readDataset("Datasets/51/51_data.txt");

    // Create a vector 2 D that stores the distances between places

    std::vector<std::vector<double>> dist = Compute_Distances_2DVector(node_vector);
    printMatrix2D(dist);

    std::vector<std::vector<int>> dist_ranking = Distance_Ranking_2DVector(dist);
    // (1) A generation with X species

    std::vector<std::vector<std::vector<int>>> species = Random_Generation(node_vector, 2, 30);
    printSpecies(species);
    std::vector<double> cost_vector_specie;
    int best_index = 0;
    int taille = 0;
	std::cout << "Species Printed" << std::endl;
    for (int i = 0; i < 10000; i++)
    {
        // Calculate Total Cost

        cost_vector_specie = Total_Cost_Specie(3, species[0], dist, dist_ranking);

        if (i % 1000 == 0)
        {
            Print_Double_Vector(cost_vector_specie);
            Print_Individual(species[0][0]);
        }

        // (2) Compute generations ; For each species, Keep X Best
        best_index = Select_Best(cost_vector_specie);

        if (i % 10000 == 0)
        {
            std::cout << "The best one is at index [" << best_index << "] " << std::endl;
        }
        // (3) From the X best: Mutate with random chances
        //Print_Individual(species[0][best_index]);

        //std::vector<int> individual = Mutation_Swap(100, species[0][best_index]);
        taille = species[0].size();

        for (int i = 0; i < taille; i++)
        {
            if (i != best_index)
            {
                species[0][i] = Mutations(5, 80, species[0][best_index]);
            }

        }

        //Print_Specie(species[0]);

        //std::cout << "kakouuuu" << std::endl;

        //Print_Individual(individual);


        // (4) Go back to square (1) OR Exit if it loops indefinetly. (Maybe store best each time in a file)
    }


}