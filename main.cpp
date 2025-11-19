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

    std::vector<std::vector<double>> dist_ranking = Distance_Ranking_2DVector(dist);
    // (1) A generation with X species

    std::vector<std::vector<std::vector<int>>> species = Random_Generation(node_vector, 2, 10);
    printSpecies(species);

    for (int i = 0; i < 1000000; i++)
    {
        // Calculate Total Cost

        std::vector<double> cost_vector_specie = Total_Cost_Specie(3, species[0], dist);
        if (i % 100000 == 0)
        {
            Print_Double_Vector(cost_vector_specie);
        }

        // (2) Compute generations ; For each species, Keep X Best
        int best_index = Select_Best(cost_vector_specie);

        if (i % 100000 == 0)
        {
            std::cout << "The best one is at index [" << best_index << "] " << std::endl;
        }
        // (3) From the X best: Mutate with random chances
        //Print_Individual(species[0][best_index]);

        //std::vector<int> individual = Mutation_Swap(100, species[0][best_index]);
        int taille = species[0].size();

        for (int i = 0; i < taille; i++)
        {
            if (i != best_index)
            {
                species[0][i] = Mutation_Swap(100, species[0][best_index]);
            }

        }

        //Print_Specie(species[0]);

        //std::cout << "kakouuuu" << std::endl;

        //Print_Individual(individual);


        // (4) Go back to square (1) OR Exit if it loops indefinetly. (Maybe store best each time in a file)
    }


}