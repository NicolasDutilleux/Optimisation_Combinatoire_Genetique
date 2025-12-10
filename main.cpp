#include "logic_function.h"
#include "hierarchy_and_print_utils.h"

int main()
{
    // Affiche le dossier courant
    current_folder_path();
    std::cout << "Hello World!\n\n";

    // 1) Chargement des données
    std::vector<Node> node_vector = readDataset("Datasets/51/51_data.txt");
    int max_id = node_vector.back().id;

    // 2) Matrice des distances + ranking
    std::vector<std::vector<double>> dist = Compute_Distances_2DVector(node_vector);
    printMatrix2D(dist);

    std::vector<std::vector<int>> dist_ranking = Distance_Ranking_2DVector(dist);

    // 3) Paramètres GA
    const int NUM_SPECIES = 3;
    const int POP_SIZE = 150;
    const int MAX_GENERATIONS = 1000;
    const int ALPHA = 3;
    double MUTATION_RATE = 0.15;   // à ajuster
    const int ELITISM = 2;

    int DEL_PCT_RDM = 10;
    int DEL_PCT_CENT = 10;
    int SWAP_PCT = 10;
    const int INV_PCT = 10;
    const int SCR_PCT = 10;
    const int INSERTSWAP_PCT = 10;
    double old_best = 1e18;

    // 4) Génération aléatoire des espèces (3 espèces)
    std::vector<std::vector<Individual>> species =
        Random_Generation(node_vector, NUM_SPECIES, POP_SIZE);

    std::cout << "Species generated\n";

    // 5) Boucle des générations
    for (int gen = 0; gen < MAX_GENERATIONS; ++gen)
    {
        // --- Évolution de chaque espèce ---
        for (int s = 0; s < NUM_SPECIES; ++s)
        {
            EvolveSpecie(
                species[s],      // la population de l'espèce s
                dist,
                dist_ranking,
                node_vector,
                ALPHA,
                MUTATION_RATE,
                max_id,
                /*elitism_count*/ ELITISM,
				DEL_PCT_RDM,
                DEL_PCT_CENT,
                SWAP_PCT,
                INV_PCT,
                SCR_PCT,
				INSERTSWAP_PCT
            );
        }

        // --- Log / visu toutes les X générations ---
        if (gen % (MAX_GENERATIONS/100) == 0)
        {
            // On cherche le meilleur individu parmi toutes les espèces
            double global_best_cost = 1e18;
            int best_species = 0;
            int best_index = 0;

            for (int s = 0; s < NUM_SPECIES; ++s)
            {
                std::vector<double> costs_s =
                    Total_Cost_Specie(ALPHA, species[s], dist, dist_ranking);

                int best_idx_s = Select_Best(costs_s);

                if (costs_s[best_idx_s] < global_best_cost)
                {
                    global_best_cost = costs_s[best_idx_s];
                    best_species = s;
                    best_index = best_idx_s;
                }

                // (optionnel) afficher les coûts de l'aespèce 0 uniquement
                /*if (s == 0)
                {
                    std::cout << "\nGeneration " << gen
                        << " - Species 0 cost vector:\n";
                    Print_Double_Vector(costs_s);
                }*/
            }

            std::cout << "\nGeneration " << gen
                << " - Global best = species " << best_species
                << ", index " << best_index
                << ", cost = " << global_best_cost << "\n";

            // Afficher le meilleur individu global (ids + mask)
            Print_Individual(species[best_species][best_index]);

            // Plot SVG en tenant compte du mask
            PlotIndividualSVG(species[best_species][best_index],
                node_vector,
                gen);
            if(global_best_cost == old_best)
                {
                DEL_PCT_RDM = std::min(DEL_PCT_RDM + 1, 20); // increase deletion percentage up to 50%
                DEL_PCT_CENT= std::min(DEL_PCT_CENT - 1, 5);
				SWAP_PCT = std::min(SWAP_PCT - 1 , 0);
				MUTATION_RATE = std::min(MUTATION_RATE + 0.03, 0.5); // increase mutation rate up to 50%
            }
            else
            {
                DEL_PCT_RDM = std::max(DEL_PCT_RDM - 1, 5); // decrease deletion percentage down to 10%
                DEL_PCT_CENT = std::min(DEL_PCT_CENT + 1, 20);
				MUTATION_RATE = std::max(MUTATION_RATE - 0.01, 0.10); // decrease mutation rate down to 5%
                SWAP_PCT = std::max(SWAP_PCT + 1 , 20);
			}
			old_best = global_best_cost;
        }
        

    }

    return 0;
}
