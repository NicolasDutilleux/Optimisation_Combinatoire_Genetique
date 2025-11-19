#include <vector>
#include <iostream>
#include <fstream>
#include "Node.h"
#include <filesystem>
#include <string>


std::vector<Node> readDataset(const std::string& filename)
{
    // Reads the file + errors
    std::vector<Node> node_vector;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: could not open file " << filename << std::endl;
        return node_vector; // return empty vector
    }
    std::string line;

    // Skip the DIMENSION line
    std::getline(file, line);

    // Skip the BEGIN line
    std::getline(file, line);

    int id;
    int x, y;

    // For each line (excluding the first): store the id, x and y in the node.

    while (file >> id >> x >> y)
    {
        Node n;
        n.id = id;
        n.x = x;
        n.y = y;

        // Add the node to the vector
        node_vector.push_back(n);
    }

    std::cout << node_vector.size() << " nodes loaded from " << filename << std::endl;
    // Close the file

    file.close();

    // Return the file

    return node_vector;
}

void printMatrix2D(const std::vector<std::vector<double>>& matrix)
{
    std::cout << "\nMatrix2D :\n\n";
    // int size_i = matrix.size();
    // 
    int size_i = 10;
    // Iterate through rows
    for (size_t i = 0; i < size_i; i++)
    {
        // Iterate through columns
        for (size_t j = 0; j < 10; j++)//matrix[i].size(); j++)
        {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << "\n"; // new line after each row
    }
}

void current_folder_path()
{
    std::cout << "\n Dossier courant = "
        << std::filesystem::current_path()
        << std::endl << std::endl;
}
void printSpecies(const std::vector<std::vector<std::vector<int>>>& species)
{
    std::cout << "\n=== Species Overview ===\n\n";

    // Loop through species
    for (size_t i = 0; i < species.size(); i++)
    {
        std::cout << "Species " << i << ":\n";

        // Loop through individuals inside a species
        for (size_t j = 0; j < species[i].size(); j++)
        {
            std::cout << "  Individual " << j << ": ";

            // Print the permutation (list of ints)
            for (size_t k = 0; k < species[i][j].size(); k++)
            {
                std::cout << species[i][j][k] << " ";
            }

            std::cout << "\n";
        }

        std::cout << "\n"; // space between species
    }
}