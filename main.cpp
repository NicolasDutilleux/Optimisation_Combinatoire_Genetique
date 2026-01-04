#include "generation/PopulationInit.h"
#include "utils/Distance.h"
#include "utils/hierarchy_and_print_utils.h"
#include "evolution/EvolveSpecie.h"
#include "cost/Cost.h"
#include "genetic/Selection.h"
#include <iostream>
#include <chrono>
#include <iomanip>

int main()
{
    // On désactive la synchro avec stdio pour que cout soit plus réactif si besoin, 
    // mais on utilisera std::endl pour flusher.
    std::ios_base::sync_with_stdio(false);

    std::cout << "[DEBUG] === PROGRAM START ===" << std::endl;

    auto start_total = std::chrono::high_resolution_clock::now();

    // Load dataset
    std::cout << "[DEBUG] Loading dataset 'data/225/225_data.txt'..." << std::endl;
    std::vector<Node> node_vector = readDataset("data/225/225_data.txt");
    int total_stations = static_cast<int>(node_vector.size());

    if (total_stations == 0) {
        std::cerr << "[ERROR] No nodes loaded!" << std::endl;
        return 1;
    }
    std::cout << "[DEBUG] Loaded " << total_stations << " nodes." << std::endl;

    // Compute distance matrix
    std::cout << "[DEBUG] Computing Distance Matrix..." << std::endl;
    std::vector<std::vector<double>> dist = Compute_Distances_2DVector(node_vector);
    std::cout << "[DEBUG] Distance Matrix Done." << std::endl;

    std::cout << "[DEBUG] Computing Ranking..." << std::endl;
    std::vector<std::vector<int>> dist_ranking = Distance_Ranking_2DVector(dist);
    std::cout << "[DEBUG] Ranking Done." << std::endl;

    // GA parameters
    const int NUM_SPECIES = 1;
    const int POP_SIZE = 200;
    const int MAX_GENERATIONS = 1;
    const int ALPHA = 3;
    double MUTATION_RATE = 0.30;
    const int ELITISM = 1;

    int ADD_PCT = 33;
    int REMOVE_PCT = 33;
    int SWAP_PCT = 20;
    const int INV_PCT = 10;
    const int SCR_PCT = 10;

    double old_best = 1e18;

    // Initialize species
    std::cout << "[DEBUG] Initializing Population (" << POP_SIZE << " individuals)..." << std::endl;
    std::vector<std::vector<Individual>> species =
        Random_Generation(node_vector, NUM_SPECIES, POP_SIZE);
    std::cout << "[DEBUG] Population Initialized." << std::endl;

    std::cout << "[DEBUG] Starting Evolution Loop..." << std::endl;

    int stagnation_count = 0;

    for (int gen = 0; gen < MAX_GENERATIONS; ++gen)
    {
        // LOG DEBUT DE GENERATION
        // On affiche tout le temps les 10 premières générations pour voir si ça démarre
        if (gen < 10 || gen % 10 == 0) {
            std::cout << "[DEBUG] Start Gen " << gen << "..." << std::endl;
        }

        for (int s = 0; s < NUM_SPECIES; ++s)
        {
            EvolveSpecie(
                species[s], dist, dist_ranking, node_vector, ALPHA,
                total_stations, MUTATION_RATE, ELITISM,
                ADD_PCT, REMOVE_PCT, SWAP_PCT, INV_PCT, SCR_PCT, 0.5);
        }

        // LOG FIN DE GENERATION (Succinct)
        if (gen < 10 || gen % 500 == 0)
        {
            // On calcule rapidement le meilleur pour l'afficher
            double current_best = species[0][0].cached_cost;
            // (suppose que EvolveSpecie a trié, le meilleur est à l'index 0 ou proche)
            // Pour être sûr on scanne vite fait :
            for (const auto& ind : species[0]) {
                if (ind.cached_cost < current_best) current_best = ind.cached_cost;
            }
            std::cout << "   -> End Gen " << gen << " | Best Cost approx: " << current_best << std::endl;
        }

        // ... (Logique de rapport complet tous les 500 gens inchangée, je l'abrège ici pour la clarté) ...
        if (gen % 500 == 0 && gen > 0)
        {
            // Votre code de reporting habituel...
            std::cout << "[REPORT] Generation " << gen << " complete." << std::endl;
        }
    }

    std::cout << "[DEBUG] Program Finished." << std::endl;
    return 0;
}