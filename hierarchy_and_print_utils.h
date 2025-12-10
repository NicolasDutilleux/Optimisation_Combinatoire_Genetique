#include <vector>
#include <iostream>
#include <fstream>
#include "Node.h"
#include <filesystem>
#include <string>
#include "Individual.h"

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
    // 1) Permutation brute
    std::cout << "Permutation (ids) : ";
    for (int id : ind.ids) {
        std::cout << id << " ";
    }
    std::cout << "\n";

    // 2) Mask par ID
    std::cout << "Mask (par ID)     : ";
    for (size_t i = 0; i < ind.mask.size(); ++i) {
        std::cout << (ind.mask[i] ? 1 : 0) << " ";
    }
    std::cout << "\n";

    // 3) Séquence active (tour effectif)
    std::cout << "Active sequence   : ";
    for (int id : ind.ids) {
        if (ind.mask[id - 1]) {
            std::cout << id << " ";
        }
    }
    std::cout << "\n\n";
}
/*void Print_Specie(std::vector<std::vector<int>> specie)
{
    int taille = specie.size();

    for (int i = 0; i < taille; i++)
    {
        std::cout << "Individual Number [" << i << "] " << std::endl;
        Print_Individual(specie[i]);
    }


}*/
void PlotIndividualSVG(const Individual& ind,
    const std::vector<Node>& node_vector,
    int generation)
{
    // Ensure folder exists
    std::filesystem::create_directories("Images");

    // === 1) Compute scaling ===
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
    double scale = 900.0 / std::max(rangeX, rangeY);  // big square
    double offset = 50.0; // margin

    auto SX = [&](double x) { return (x - minX) * scale + offset; };
    auto SY = [&](double y) { return (y - minY) * scale + offset; };

    // === 2) Build active sequence from ids + mask ===
    std::vector<int> active;
    active.reserve(ind.ids.size());
    for (int id : ind.ids) {
        if (ind.mask[id - 1]) {
            active.push_back(id);
        }
    }

    std::string filename = "Images/plot_individual_" +
        std::to_string(generation) + ".svg";

    std::ofstream out(filename);
    if (!out)
    {
        std::cerr << "Cannot write SVG file: " << filename << "\n";
        return;
    }

    out << "<svg xmlns='http://www.w3.org/2000/svg' width='1000' height='1000'>\n";
    out << "<rect width='100%' height='100%' fill='white'/>\n";

    // === 3) Draw all points (background) ===
    for (const Node& n : node_vector)
    {
        out << "<circle cx='" << SX(n.x)
            << "' cy='" << SY(n.y)
            << "' r='4' fill='lightgray'/>\n";
    }

    // === 4) Draw active path (lines) ===
    if (active.size() >= 2) {
        // lines between successive active stations
        for (size_t i = 0; i + 1 < active.size(); ++i)
        {
            const Node& a = node_vector[active[i] - 1];
            const Node& b = node_vector[active[i + 1] - 1];

            out << "<line x1='" << SX(a.x) << "' y1='" << SY(a.y)
                << "' x2='" << SX(b.x) << "' y2='" << SY(b.y)
                << "' stroke='red' stroke-width='2'/>\n";
        }

        // close loop : last -> first
        const Node& a = node_vector[active.back() - 1];
        const Node& b = node_vector[active.front() - 1];

        out << "<line x1='" << SX(a.x) << "' y1='" << SY(a.y)
            << "' x2='" << SX(b.x) << "' y2='" << SY(b.y)
            << "' stroke='red' stroke-width='2'/>\n";
    }

    // === 5) Draw active points ===
    for (int id : active)
    {
        const Node& n = node_vector[id - 1];
        if (id == 1) {
            // depot / station 1 en bleu
            out << "<circle cx='" << SX(n.x)
                << "' cy='" << SY(n.y)
                << "' r='10' fill='blue'/>\n";
        }
        else {
            out << "<circle cx='" << SX(n.x)
                << "' cy='" << SY(n.y)
                << "' r='7' fill='red'/>\n";
        }
    }

    out << "</svg>\n";
    out.close();

    std::cout << "SVG plot generated: " << filename << "\n";
}