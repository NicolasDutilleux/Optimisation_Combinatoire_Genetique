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
    auto start_total = std::chrono::high_resolution_clock::now();
    
    std::cout << "[MAIN] Starting program...\n";
    
    // Load dataset
    std::cout << "[MAIN] Loading dataset...\n";
    std::vector<Node> node_vector = readDataset("data/225/225_data.txt");
    int total_stations = static_cast<int>(node_vector.size());

    if (total_stations == 0)
    {
        std::cerr << "Error: No nodes loaded\n";
        return 1;
    }
    std::cout << "[MAIN] Loaded " << total_stations << " nodes\n";

    // Compute distance matrix and ranking
    std::cout << "[MAIN] Computing distance matrix...\n";
    auto start_dist = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<double>> dist = Compute_Distances_2DVector(node_vector);
    auto end_dist = std::chrono::high_resolution_clock::now();
    auto dist_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_dist - start_dist).count();
    std::cout << "[MAIN] Distance matrix computed in " << dist_time << "ms\n";

    std::cout << "[MAIN] Computing distance ranking...\n";
    auto start_rank = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<int>> dist_ranking = Distance_Ranking_2DVector(dist);
    auto end_rank = std::chrono::high_resolution_clock::now();
    auto rank_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_rank - start_rank).count();
    std::cout << "[MAIN] Distance ranking computed in " << rank_time << "ms\n";

    // GA parameters - TUNED FOR PERFORMANCE
    const int NUM_SPECIES = 3;
    const int POP_SIZE = 100;          // Increased from 20 for better quality
    const int MAX_GENERATIONS = 10000; // Reduced from 100,000 for reasonable runtime
    const int ALPHA = 3;
    double MUTATION_RATE = 0.20;       // Slightly increased for exploration
    const int ELITISM = 3;

    int ADD_PCT = 20;     // Increased for larger exploration
    int REMOVE_PCT = 15;
    int SWAP_PCT = 15;
    const int INV_PCT = 10;
    const int SCR_PCT = 10;
    double old_best = 1e18;

    // Initialize species
    std::cout << "[MAIN] Initializing " << NUM_SPECIES << " species with " << POP_SIZE << " individuals...\n";
    auto start_init = std::chrono::high_resolution_clock::now();
    std::vector<std::vector<Individual>> species =
        Random_Generation(node_vector, NUM_SPECIES, POP_SIZE);
    auto end_init = std::chrono::high_resolution_clock::now();
    auto init_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_init - start_init).count();
    std::cout << "[MAIN] Species initialized in " << init_time << "ms\n";

    std::cout << "[MAIN] Starting evolution loop...\n";

    // Main evolution loop
    int stagnation_count = 0;
    const int STAGNATION_LIMIT = 1000;  // Stop if no improvement for 1000 generations
    
    for (int gen = 0; gen < MAX_GENERATIONS; ++gen)
    {
        // Evolve each species
        for (int s = 0; s < NUM_SPECIES; ++s)
        {
            EvolveSpecie(
                species[s],
                dist,
                dist_ranking,
                node_vector,
                ALPHA,
                total_stations,
                MUTATION_RATE,
                ELITISM,
                ADD_PCT,
                REMOVE_PCT,
                SWAP_PCT,
                INV_PCT,
                SCR_PCT,
                0.5);
        }

        // Log every 500 generations (reduced from 100)
        if (gen % 500 == 0 && gen > 0)
        {
            std::cout << "\n" << std::string(70, '=') << "\n";
            std::cout << "GENERATION " << std::setw(6) << gen << " REPORT\n";
            std::cout << std::string(70, '=') << "\n";

            double global_best_cost = 1e18;
            int best_species_global = 0;
            int best_index_global = 0;

            // Report for each species
            for (int s = 0; s < NUM_SPECIES; ++s)
            {
                std::vector<double> costs_s =
                    Total_Cost_Specie(ALPHA, species[s], total_stations, dist, dist_ranking);

                int best_idx_s = Select_Best(costs_s);
                double best_cost_s = costs_s[best_idx_s];
                
                // Calculate statistics
                double avg_cost = 0.0;
                double min_cost = 1e18;
                double max_cost = -1e18;
                
                for (double c : costs_s)
                {
                    avg_cost += c;
                    min_cost = std::min(min_cost, c);
                    max_cost = std::max(max_cost, c);
                }
                avg_cost /= costs_s.size();

                const Individual& best_ind = species[s][best_idx_s];
                
                std::cout << "\n[SPECIES " << s << "]\n";
                std::cout << "  Best Cost:     " << std::fixed << std::setprecision(2) << best_cost_s << "\n";
                std::cout << "  Avg Cost:      " << std::fixed << std::setprecision(2) << avg_cost << "\n";
                std::cout << "  Ring Size:     " << best_ind.active_ring.size() << "/" << total_stations << "\n";

                if (best_cost_s < global_best_cost)
                {
                    global_best_cost = best_cost_s;
                    best_species_global = s;
                    best_index_global = best_idx_s;
                }
            }

            std::cout << "\n[GLOBAL BEST]\n";
            std::cout << "  Cost:          " << std::fixed << std::setprecision(2) << global_best_cost << "\n";
            std::cout << "  Species:       " << best_species_global << "\n";
            std::cout << std::string(70, '=') << "\n\n";

            // Check for improvement
            if (global_best_cost < old_best)
            {
                stagnation_count = 0;
                std::cout << "  [NEW BEST] Cost improved!\n\n";
            }
            else
            {
                stagnation_count++;
                if (stagnation_count > 2)  // Stop after 2 reports without improvement (~1000 gens)
                {
                    std::cout << "  [STAGNATION] Stopping early...\n\n";
                    break;
                }
            }
            old_best = global_best_cost;
        }
    }

    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "FINAL RESULTS\n";
    std::cout << std::string(70, '=') << "\n";

    double final_best_cost = 1e18;
    int final_best_species = 0;
    int final_best_index = 0;

    for (int s = 0; s < NUM_SPECIES; ++s)
    {
        std::vector<double> costs_s =
            Total_Cost_Specie(ALPHA, species[s], total_stations, dist, dist_ranking);

        int best_idx_s = Select_Best(costs_s);

        if (costs_s[best_idx_s] < final_best_cost)
        {
            final_best_cost = costs_s[best_idx_s];
            final_best_species = s;
            final_best_index = best_idx_s;
        }
    }

    std::cout << "Best cost:     " << std::fixed << std::setprecision(2) << final_best_cost << "\n";
    std::cout << "Best species:  " << final_best_species << "\n";
    std::cout << "Ring size:     " << species[final_best_species][final_best_index].active_ring.size() << "/" << total_stations << "\n";
    std::cout << "Ring:          ";
    for (int id : species[final_best_species][final_best_index].active_ring)
        std::cout << id << " ";
    std::cout << "\n";

    auto end_total = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::seconds>(end_total - start_total).count();
    std::cout << "\nTotal time:    " << total_time << " seconds\n";
    std::cout << std::string(70, '=') << "\n";
    std::cout << std::flush;

    return 0;
}
