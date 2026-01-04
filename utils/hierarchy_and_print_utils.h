#include <vector>
#include <iostream>
#include <fstream>
#include "core/Node.h"
#include <filesystem>
#include <string>
#include "core/Individual.h"

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

void Print_Double_Vector(std::vector<double> cost_vector)
{
    std::cout << cost_vector.size() << std::endl;
    for (int i = 0; i < cost_vector.size(); i++)
    {
        std::cout << "Total cost " << i << " = " << cost_vector[i] << "       " << i << "  \n";
    }
}

void Print_Individual(const Individual& ind)
{
    // Print active ring only (no mask needed)
    std::cout << "Active ring: ";
    for (int id : ind.active_ring) {
        std::cout << id << " ";
    }
    std::cout << "\n";
}

void PlotIndividualSVG(const Individual& ind,
    const std::vector<Node>& node_vector,
    int generation,
    std::vector<int> assignement)
{
    std::filesystem::create_directories("Images");

    if (node_vector.empty() || ind.active_ring.empty()) return;

    int minX = node_vector[0].x, maxX = node_vector[0].x;
    int minY = node_vector[0].y, maxY = node_vector[0].y;

    for (const Node& n : node_vector)
    {
        minX = std::min(minX, n.x);
        maxX = std::max(maxX, n.x);
        minY = std::min(minY, n.y);
        maxY = std::max(maxY, n.y);
    }

    double rangeX = maxX - minX;
    double rangeY = maxY - minY;
    if (rangeX == 0) rangeX = 1.0;
    if (rangeY == 0) rangeY = 1.0;
    
    double scale = 900.0 / std::max(rangeX, rangeY);
    double offset = 50.0;

    auto SX = [&](double x) { return (x - minX) * scale + offset; };
    auto SY = [&](double y) { return (y - minY) * scale + offset; };

    const std::vector<int>& active = ind.active_ring;

    std::string filename = "Images/plot_individual_" + std::to_string(generation) + ".svg";
    std::ofstream out(filename);
    if (!out)
    {
        std::cerr << "Cannot write SVG file: " << filename << "\n";
        return;
    }

    out << "<svg xmlns='http://www.w3.org/2000/svg' width='1000' height='1000'>\n";
    out << "<rect width='100%' height='100%' fill='white'/>\n";

    for (const Node& n : node_vector)
    {
        out << "<circle cx='" << SX(n.x) << "' cy='" << SY(n.y) << "' r='4' fill='brown'/>\n";
    }

    if (active.size() >= 2)
    {
        for (size_t i = 0; i + 1 < active.size(); ++i)
        {
            int idx_a = active[i] - 1;
            int idx_b = active[i + 1] - 1;
            if (idx_a >= 0 && idx_a < (int)node_vector.size() &&
                idx_b >= 0 && idx_b < (int)node_vector.size())
            {
                const Node& a = node_vector[idx_a];
                const Node& b = node_vector[idx_b];
                out << "<line x1='" << SX(a.x) << "' y1='" << SY(a.y)
                    << "' x2='" << SX(b.x) << "' y2='" << SY(b.y)
                    << "' stroke='red' stroke-width='2'/>\n";
            }
        }

        for (size_t i = 0; i + 1 < assignement.size(); i += 2)
        {
            int idx_a = assignement[i] - 1;
            int idx_b = assignement[i + 1] - 1;
            if (idx_a >= 0 && idx_a < (int)node_vector.size() &&
                idx_b >= 0 && idx_b < (int)node_vector.size())
            {
                const Node& a = node_vector[idx_a];
                const Node& b = node_vector[idx_b];
                out << "<line x1='" << SX(a.x) << "' y1='" << SY(a.y)
                    << "' x2='" << SX(b.x) << "' y2='" << SY(b.y)
                    << "' stroke='blue' stroke-width='2'/>\n";
            }
        }

        int idx_a = active.back() - 1;
        int idx_b = active.front() - 1;
        if (idx_a >= 0 && idx_a < (int)node_vector.size() &&
            idx_b >= 0 && idx_b < (int)node_vector.size())
        {
            const Node& a = node_vector[idx_a];
            const Node& b = node_vector[idx_b];
            out << "<line x1='" << SX(a.x) << "' y1='" << SY(a.y)
                << "' x2='" << SX(b.x) << "' y2='" << SY(b.y)
                << "' stroke='red' stroke-width='2'/>\n";
        }
    }

    for (int id : active)
    {
        int idx = id - 1;
        if (idx >= 0 && idx < (int)node_vector.size())
        {
            const Node& n = node_vector[idx];
            if (id == 1)
            {
                out << "<circle cx='" << SX(n.x) << "' cy='" << SY(n.y)
                    << "' r='10' fill='blue'/>\n";
            }
            else
            {
                out << "<circle cx='" << SX(n.x) << "' cy='" << SY(n.y)
                    << "' r='7' fill='red'/>\n";
            }
        }
    }

    out << "</svg>\n";
    out.close();

    std::cout << "SVG plot generated: " << filename << "\n";
}