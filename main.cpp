#include "logic_function.h"
#include "hierarchy_and_print_utils.h"


int main()
{

    current_folder_path();

	std::cout << "Hello World!\n\n";
    // LOAD DATA FROM NOTEBOOK

    std::vector<Node> node_vector = readDataset("Datasets/51/51_data.txt");

    // Create a vector 2 D that stores the distances between places

    std::vector<std::vector<double>> dist = Compute_Distances_2DVector(node_vector);

    // (1) A generation with X species

    printMatrix2D(dist);

        // (2) Compute generations ; For each species, Keep X Best

        // (3) From the X best: Mutate with random chances

        // (4) Go back to square (1) OR Exit if it loops indefinetly. (Maybe store best each time in a file)

}