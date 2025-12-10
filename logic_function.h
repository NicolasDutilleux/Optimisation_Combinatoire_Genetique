#include <vector>
#include <cmath>
#include <random>
#include "Node.h"
#include "Individual.h"
#include <iostream>

// ------------------- //
// Utility Functions //
// ------------------- //

/*int Random(int minimum, int maximum)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(minimum, maximum);

    return distr(gen);
}*/ // Depreciated because of global RNG below : It makes one distribution and uses this each time : less costly

inline std::mt19937& GLOBAL_RNG() {
    static std::random_device rd;
    static std::mt19937 g(rd());
    return g;
}
inline int RandInt(int a, int b) {
    std::uniform_int_distribution<int> d(a, b);
    return d(GLOBAL_RNG());
}
inline double RandDouble() {
    std::uniform_real_distribution<double> d(0.0, 1.0);
    return d(GLOBAL_RNG());
}

// ------------------- //
// Generation Functions //
// ------------------- //

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
void Bubble_Sort(std::vector<int>& vector_int_to_sort, std::vector<double>& vector_double_to_sort)
{
    int taille = vector_int_to_sort.size();
    bool swapped = true;
    int swap_int = 0;
    double swap_double = 0;
    while (swapped)
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
           //std::cout << vector_int_to_sort[0] << std::endl;
        }
       
    }


}
std::vector<std::vector<int>> Distance_Ranking_2DVector(std::vector<std::vector<double>> distance_vector)
{
	std::cout<<"INNN" << std::endl;
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
        std::cout << "Out " << i << std::endl;
    }
    std::cout << "Out" << std::endl;
	return ranking_vector2D;
}


std::vector<std::vector<Individual>> Random_Generation(const std::vector<Node>& node_vector,
    int species_number,
    int individual_number)
{
    std::vector<std::vector<Individual>> species(species_number);
    std::random_device rd;
    std::mt19937 rng(rd());

    int station_number = node_vector.back().id;

    // Construire le pool en commençant par 1
    std::vector<int> pool;
    pool.reserve(station_number);
	pool.push_back(1); // keep id 1 at the start
    for (int x = 2; x <= station_number; x++)
    {
        pool.push_back(x);
    }

    std::cout << "Size is " << pool.size() << std::endl;

    for (int i = 0; i < species_number; i++)
    {
        for (int j = 0; j < individual_number; j++)
        {
            // Copie du pool
            std::vector<int> individual = pool;
            Individual ind;
            // Mélanger tout sauf la première position (qui reste 1)
            std::shuffle(individual.begin() + 1, individual.end(), rng);

			ind.ids = individual; // Les Ids dans un certain ordre : le masque dira quelles stations sont visitées
            ind.mask = std::vector<bool>(station_number, true); // tout visité au début le mask ne contien que des "1"

            

            species[i].push_back(ind);
        }
    }

    return species;
}

// TO SWAPPPPP
std::vector<std::vector<std::vector<int>>> Informed_Generation(const std::vector<Node>& node_vector,
    int species_number,
    int individual_number,
    std::vector<std::vector<int>>& ranking_vector
)
{
	return std::vector<std::vector<std::vector<int>>>();
}

// ------------------- //
// Cost Functions //
// ------------------- //

double Cost_station(int alpha, double distance)
{
    double cost = alpha * distance;

    return cost;
}

double Cost_out_ring(int alpha, double distance)
{
    double cost = (10 - alpha) * distance;

    return cost;
}
int Find_Nearest_Station(int city_id,
    const std::vector<bool>& mask,
    const std::vector<std::vector<int>>& ranking_vector)
{
    // Rangée triée par distance croissante à city_id
    const std::vector<int>& row = ranking_vector[city_id - 1];

    for (int candidate_id : row) {
        // mask indexe par ID - 1
        if (mask[candidate_id - 1]) {
            return candidate_id;
        }
    }

    return -1; // aucun actif trouvé (ne devrait pas arriver si au moins une station active)
}

double Total_Cost_Individual(
    int alpha,
    const std::vector<int>& perm,
    const std::vector<bool>& mask,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector)
{
    int taille = perm.size();
    int number_of_stations = dist.size();
    double total_cost = 0.0;

    // 1) trouver la première station active
    int first_idx = -1;
    for (int i = 0; i < taille; ++i) {
        if (mask[perm[i] - 1]) {
            first_idx = i;
            break;
        }
    }
    if (first_idx == -1) return 1e18; // aucune station active

    int prev_idx = first_idx;
    int prev_id = perm[prev_idx];

    // 2) parcourir tout le tour en reliant les stations actives successives
    for (int step = 1; step < taille; ++step) {
        int i = (first_idx + step) % taille; // maybe logic error if the active first station isn't 0
        int id = perm[i];

        if (!mask[id - 1]) continue; // inactive, on saute

        // relier prev_id -> id
        total_cost += Cost_station(alpha, dist[prev_id - 1][id - 1]);
        prev_idx = i;
        prev_id = id;
    }

    // 3) fermer la boucle : dernière active -> première active
    int first_id = perm[first_idx];
    if (prev_id != first_id) {
        total_cost += Cost_station(alpha, dist[prev_id - 1][first_id - 1]);
    }

    // 4) pénalité pour stations non visitées (mask = false)
    for (int s = 1; s <= number_of_stations; ++s) {
        if (!mask[s - 1]) {
            int nearest_station_id = Find_Nearest_Station(s, mask, ranking_vector);
            double distance_station = dist[s - 1][nearest_station_id - 1];
            total_cost += Cost_out_ring(alpha, distance_station);
        }
    }

    return total_cost;
}



std::vector<double> Total_Cost_Specie(
    int alpha,
    std::vector<Individual>& specie,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking)
{

	// Return a vector of total costs for each individual in the specie //

    int taille = specie.size();
    std::vector<double> cost_vector_specie;
    cost_vector_specie.reserve(taille);

    for (int i = 0; i < taille; ++i) {
        cost_vector_specie.push_back(
            Total_Cost_Individual(alpha,
                specie[i].ids,
                specie[i].mask,
                dist,
                ranking));
    }
    return cost_vector_specie;
}


// ------------------- //
// Selection Functions //
// ------------------- //

int Select_Best(std::vector<double> cost_vector_specie)
{

	// Return the index of the best individual in the specie //

    int taille = cost_vector_specie.size();
    int best_index = 0;
    double actual_best = cost_vector_specie[0];
    for (int i = 0; i < taille; i++)
    {
        if (cost_vector_specie[i] < actual_best)
        {
            actual_best = cost_vector_specie[i];
            best_index = i;
        }

    }
    return best_index;
}

// ------------------- //
// Mutation Functions //
// ------------------- //

inline void Mutation_Swap_Simple(std::vector<int>& individual) {

	// Swap two random positions in the individual: No heuristic //

    int n = static_cast<int>(individual.size());
    int a = RandInt(1, n - 1); // preserve depot if that's the rule
    int b = RandInt(1, n - 1);
    std::swap(individual[a], individual[b]);
}

inline void Mutation_Inversion(std::vector<int>& individual) {

	// Invert a random segment of the individual : no heuristic but a lot of time segment are better //

    int n = static_cast<int>(individual.size());
    int a = RandInt(1, n - 1);
    int b = RandInt(1, n - 1);
    if (a > b) std::swap(a, b);
    std::reverse(individual.begin() + a, individual.begin() + b + 1);
}
inline void Mutation_Scramble(std::vector<int>& individual) {

	// Scramble a random segment of the individual : no heuristic but a lot of time segment are better //

    int n = static_cast<int>(individual.size());
    int a = RandInt(1, n - 1);
    int b = RandInt(1, n - 1);
    if (a > b) std::swap(a, b);
    std::shuffle(individual.begin() + a, individual.begin() + b + 1, GLOBAL_RNG());
}

inline void Mutation_Insert_Swap(std::vector<int>& individual) {

	// Remove an element at position a and insert it at position b  //
       /* Effect:
        -element at a is removed
        - elements a + 1 ... b shift left
        - val is inserted at position */

    int n = static_cast<int>(individual.size());
    int a = RandInt(1, n - 1);
    int b = RandInt(1, n - 1);
    if (a == b) return;
    int val = individual[a];
    if (a < b) {
        std::rotate(individual.begin() + a, individual.begin() + a + 1, individual.begin() + b + 1);
    }
    else {
        std::rotate(individual.begin() + b, individual.begin() + a, individual.begin() + a + 1);
    }
}

inline void Mutation_Deletion_Centroid(
    std::vector<int>& ids,
    std::vector<bool>& mask,
    const std::vector<Node>& nodes)
{

	// Deactivate one station based on centroid heuristic //
    

    if (std::count(mask.begin(), mask.end(), true) <= 2) return;

    // ---- 1. compute centroid of ACTIVE stations only ----
    double cx = 0.0, cy = 0.0;
    int activeCount = 0;

    for (int id = 1; id <= (int)mask.size(); id++) {
        if (mask[id - 1]) {
            const Node& nd = nodes[id - 1];
            cx += nd.x;
            cy += nd.y;
            activeCount++;
        }
    }

    cx /= activeCount;
    cy /= activeCount;

    // ---- 2. find ACTIVE station farthest from centroid (skip perm[0]) ----
    int best_index = -1;
    double best_dist = -1.0;

    for (int i = 1; i < (int)ids.size(); i++) {
        int station = ids[i];
        if (!mask[station - 1]) continue; // ignore inactive

        const Node& nd = nodes[station - 1];
        double dx = nd.x - cx;
        double dy = nd.y - cy;
        double d = dx * dx + dy * dy;

        if (d > best_dist) {
            best_dist = d;
            best_index = i;
        }
    }

    if (best_index <= 0) return;

    // ---- 3. deactivate this station ----
    int station_to_delete = ids[best_index];
    mask[station_to_delete - 1] = false;

    // no erase : perm remains same size
}


inline void Mutation_Insertion_Heuristic(
    std::vector<int>& ids,
    std::vector<bool>& mask,
    int max_station_id,
    const std::vector<std::vector<double>>& dist)
{
    int n = static_cast<int>(ids.size());

    // 1. Pick a station that is currently NOT active
    int station_to_insert = RandInt(2, max_station_id);
    if (mask[station_to_insert - 1])
        return; // déjà active → rien à faire

    // 2. Compute best insertion position (min added cost) sur la permutation ids
    int best_pos = 1;
    double best_cost = 1e18;

    for (int pos = 1; pos < n; ++pos) {
        int prev = ids[pos - 1];
        int next = ids[pos];

        double added =
            dist[prev - 1][station_to_insert - 1] +
            dist[station_to_insert - 1][next - 1] -
            dist[prev - 1][next - 1];

        if (added < best_cost) {
            best_cost = added;
            best_pos = pos;
        }
    }

    // 3. Activer la station dans le mask
    mask[station_to_insert - 1] = true;

    // 4. Déplacer station_to_insert vers best_pos en préservant la permutation (swap simple)
    int old_pos = -1;
    for (int i = 0; i < n; ++i) {
        if (ids[i] == station_to_insert) {
            old_pos = i;
            break;
        }
    }
    if (old_pos == -1) return; // ne devrait jamais arriver

    // On rapproche station_to_insert de la "meilleure" position
    std::swap(ids[old_pos], ids[best_pos]);
}



Individual Mutations(
    int deletion_pourcentage,
    int swapping_pourcentage,
    int inversion_pourcentage,
    int scramble_pourcentage,
    int insertswap_pourcentage,
    const Individual& individual,
    int max_station_id,
    const std::vector<Node>& nodes,
    const std::vector<std::vector<double>>& dist)
{
    Individual ind = individual;

    if (RandInt(1, 100) <= deletion_pourcentage)
        Mutation_Deletion_Centroid(ind.ids, ind.mask, nodes);

    if (RandInt(1, 100) <= swapping_pourcentage)
        Mutation_Swap_Simple(ind.ids);

    if (RandInt(1, 100) <= inversion_pourcentage)
        Mutation_Inversion(ind.ids);

    if (RandInt(1, 100) <= scramble_pourcentage)
        Mutation_Scramble(ind.ids);

    if (RandInt(1, 100) <= insertswap_pourcentage)
        Mutation_Insert_Swap(ind.ids);

    // correct call: ids + mask
    Mutation_Insertion_Heuristic(ind.ids,
                                 ind.mask,
                                 max_station_id,
                                 dist);

    return ind;
}



// ------------------- //
// Heuristics Functions //
// ------------------- //

