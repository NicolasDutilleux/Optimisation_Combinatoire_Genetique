#include <vector>
#include <cmath>
#include <random>
#include "Node.h"
#include "Individual.h"
#include <iostream>
#include <chrono>

using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;

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


std::vector<std::vector<Individual>> Random_Generation(
    const std::vector<Node>& node_vector,
    int species_number,
    int individual_number)
{
    std::vector<std::vector<Individual>> species(species_number);
    std::random_device rd;
    std::mt19937 rng(rd());

    int station_number = node_vector.back().id;

    // Construire le pool d’IDs (1…N)
    std::vector<int> pool;
    pool.reserve(station_number);
    for (int x = 1; x <= station_number; ++x)
        pool.push_back(x);

    std::cout << "Size is " << pool.size() << std::endl;

    // Pour chaque espèce et chaque individu
    for (int i = 0; i < species_number; ++i)
    {
        for (int j = 0; j < individual_number; ++j)
        {
            Individual ind;

            // --- 1. Mélange des IDs (permutation complète) ---
            ind.ids = pool;
            std::shuffle(ind.ids.begin() + 1, ind.ids.end(), rng); // garder le 1 en tête

            // --- 2. Initialisation du masque ---
            ind.mask = std::vector<bool>(station_number, false);

            // Le dépôt (id=1) doit toujours être actif
            ind.mask[0] = true;

            // Tirer un nombre aléatoire de stations actives entre 3 et station_number
            int active_count = RandInt(3, station_number);

            // Activer aléatoirement (active_count - 1) autres stations (car 1 déjà actif)
            std::vector<int> candidate_indices;
            for (int k = 1; k < station_number; ++k)
                candidate_indices.push_back(k);

            std::shuffle(candidate_indices.begin(), candidate_indices.end(), rng);
            for (int k = 0; k < active_count - 1; ++k)
                ind.mask[candidate_indices[k]] = true;

            // Ajouter l’individu à l’espèce
            species[i].push_back(std::move(ind));
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
std::vector<int> Assign_Stations(int number_of_stations, const std::vector<bool>& mask, const std::vector<std::vector<double>>& dist, const std::vector<std::vector<int>>& ranking_vector)
{
	std::vector<int> assignment;
    for (int s = 1; s <= number_of_stations; ++s) {
        if (!mask[s - 1]) {
            int nearest_station_id = Find_Nearest_Station(s, mask, ranking_vector);
            assignment.push_back(s);
            assignment.push_back(nearest_station_id);
        }
    }
	return assignment;
    
}
// Coût de l’anneau seulement (stations actives, ordre donné par perm)
inline double RingCostOnly(
    int alpha,
    const std::vector<int>& perm,
    const std::vector<bool>& mask,
    const std::vector<std::vector<double>>& dist)
{
    int n = static_cast<int>(perm.size());
    if (n <= 1) return 0.0;

    int first_idx = -1;
    for (int i = 0; i < n; ++i) {
        if (mask[perm[i] - 1]) {
            first_idx = i;
            break;
        }
    }
    if (first_idx == -1) return 1e18; // aucune station active

    double cost = 0.0;
    int prev_id = perm[first_idx];

    for (int step = 1; step < n; ++step) {
        int idx = (first_idx + step) % n;
        int id = perm[idx];
        if (!mask[id - 1]) continue;

        cost += Cost_station(alpha, dist[prev_id - 1][id - 1]);
        prev_id = id;
    }

    int first_id = perm[first_idx];
    if (prev_id != first_id) {
        cost += Cost_station(alpha, dist[prev_id - 1][first_id - 1]);
    }
    return cost;
}

// Coût hors anneau seulement (ne dépend que du mask, pas de l’ordre)
inline double OutRingCostOnly(
    int alpha,
    const std::vector<bool>& mask,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector)
{
    int number_of_stations = static_cast<int>(dist.size());
    double total_cost = 0.0;

    for (int s = 1; s <= number_of_stations; ++s) {
        if (!mask[s - 1]) {
            int nearest_station_id = Find_Nearest_Station(s, mask, ranking_vector);
            double distance_station = dist[s - 1][nearest_station_id - 1];
            total_cost += Cost_out_ring(alpha, distance_station);
        }
    }
    return total_cost;
}

inline double Total_Cost_Individual(
    int alpha,
    const std::vector<int>& perm,
    const std::vector<bool>& mask,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking_vector)
{
    double ring_cost = RingCostOnly(alpha, perm, mask, dist);
    double out_ring = OutRingCostOnly(alpha, mask, dist, ranking_vector);
    return ring_cost + out_ring;
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

// Supprime aléatoirement entre 1 et max_del stations actives (hors dépôt),
// en gardant au moins min_active stations actives au total.
inline void Mutation_RandomMultiDeletion(
    const std::vector<int>& ids,
    std::vector<bool>& mask,
    int max_del,
    int min_active = 2)
{
    int n = static_cast<int>(ids.size());

    // Nombre de stations actives actuelles
    int active_count = 0;
    for (bool b : mask) if (b) ++active_count;

    if (active_count <= min_active) return;

    // On ne peut pas supprimer plus que (active_count - min_active)
    int max_possible_del = std::min(max_del, active_count - min_active);
    if (max_possible_del <= 0) return;

    // Nombre effectif de deletions cette fois
    int k_del = RandInt(1, max_possible_del);

    // Construire la liste des positions (indices dans ids) qui sont actives, hors dépôt
    std::vector<int> active_positions;
    active_positions.reserve(n);

    for (int i = 1; i < n; ++i)  // i = 0 => dépôt, on ne le touche pas
    {
        int station_id = ids[i];         // l'ID de la station à cette position
        if (mask[station_id - 1])       // active ?
            active_positions.push_back(i);
    }

    if (active_positions.empty()) return;

    // On mélange et on choisit les k_del premières pour les désactiver
    std::shuffle(active_positions.begin(), active_positions.end(), GLOBAL_RNG());

    for (int i = 0; i < k_del && i < (int)active_positions.size(); ++i)
    {
        int pos = active_positions[i];
        int station_id = ids[pos];
        mask[station_id - 1] = false;
    }
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
    int multi_deletion_pourcentage,
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

    // 1) Suppression "intelligente" : l'outlier vis-à-vis du centroïde
    if (RandInt(1, 100) <= deletion_pourcentage)
        Mutation_Deletion_Centroid(ind.ids, ind.mask, nodes);

    // 2) Multi-suppression random : pour vraiment explorer
    if (RandInt(1, 100) <= multi_deletion_pourcentage)
        Mutation_RandomMultiDeletion(ind.ids, ind.mask,
            /*max_del=*/3, /*min_active=*/3);

    // 3) Mutations sur la permutation
    if (RandInt(1, 100) <= swapping_pourcentage)
        Mutation_Swap_Simple(ind.ids);

    if (RandInt(1, 100) <= inversion_pourcentage)
        Mutation_Inversion(ind.ids);

    if (RandInt(1, 100) <= scramble_pourcentage)
        Mutation_Scramble(ind.ids);

    if (RandInt(1, 100) <= insertswap_pourcentage)
        Mutation_Insert_Swap(ind.ids);

    // 4) Insertion heuristique MAIS PAS obligatoire :
    //    => mets-la aussi sous proba, sinon tu annules souvent les deletions
    if (RandInt(1, 100) <= 30)  // par exemple 30% de chance
    {
        Mutation_Insertion_Heuristic(ind.ids,
            ind.mask,
            max_station_id,
            dist);
    }

    return ind;
}



inline std::vector<int> Order_Crossover(const std::vector<int>& parentA, const std::vector<int>& parentB) {
	if (!(parentA.size() == parentB.size())) return std::vector<int>();
    int n = static_cast<int>(parentA.size());
    std::vector<int> child(n, -1);

    int cut1 = RandInt(1, n - 2);
    int cut2 = RandInt(cut1 + 1, n - 1);

    for (int i = cut1; i <= cut2; ++i) child[i] = parentA[i];

    int idx = (cut2 + 1) % n;
    for (int i = 0; i < n; ++i) {
        int candidate = parentB[(cut2 + 1 + i) % n];
        bool present = false;

        // linear search ok for moderate n; could optimize with boolean map
        for (int x = 0; x < n; ++x) if (child[x] == candidate) { present = true; break; }
        if (!present) {
            child[idx] = candidate;
            idx = (idx + 1) % n;
        }
    }
    // final safety: if any -1 remains (shouldn't), fill with missing genes
    for (int i = 0; i < n; ++i) {
        if (child[i] == -1) {
            for (int g = 1; g <= n; ++g) {
                bool found = false;
                for (int x = 0; x < n; ++x) if (child[x] == g) { found = true; break; }
                if (!found) { child[i] = g; break; }
            }
        }
    }
    return child;
}

inline int Tournament_Select_Index(const std::vector<double>& costs, int k)
{
    int n = static_cast<int>(costs.size());
    int best = RandInt(0, n - 1);

    for (int t = 1; t < k; ++t)
    {
        int cand = RandInt(0, n - 1);
        if (costs[cand] < costs[best])
            best = cand;
    }
    return best;
}
// Calcule le coût du tour fermé défini par 'active_ids' (toutes les stations sont actives).
// Ici on ne tient compte QUE du coût "sur l'anneau" (pas des pénalités hors anneau).
inline double RingCostActiveTour(
    int alpha,
    const std::vector<int>& active_ids,
    const std::vector<std::vector<double>>& dist)
{
    int m = static_cast<int>(active_ids.size());
    if (m <= 1) return 0.0;

    double cost = 0.0;
    for (int i = 0; i < m; ++i)
    {
        int a = active_ids[i];
        int b = active_ids[(i + 1) % m]; // tour fermé

        cost += Cost_station(alpha, dist[a - 1][b - 1]);
    }
    return cost;
}


// 2-opt optimisé sur l'anneau des stations ACTIVES.
//
// Idée :
//  - on extrait d'abord le tour des stations actives dans l'ordre de ind.ids
//  - on applique 2-opt classique sur ce tour (O(m^2) avec update de coût en O(1))
//  - à la fin, on réinjecte ce nouvel ordre actif dans ind.ids
//  - le mask NE CHANGE PAS, donc le coût "hors anneau" reste constant et n'est jamais recalculé.
//
// Par rapport à ta version précédente :
//  - on ne crée plus un "Individual candidate" à chaque (i, j) -> beaucoup moins de copies
//  - on n'appelle plus Total_Cost_Individual à l'intérieur de la double boucle
//    (donc plus de O(n) * O(n^2) = O(n^3) par itération) : on utilise une formule Δ-coût locale
//  - on travaille sur m = nb de stations actives (souvent << n) au lieu de n villes.
inline void TwoOptImproveAlpha(Individual& ind,
    int alpha,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking)
{
    const int n = static_cast<int>(ind.ids.size());
    if (n <= 3) return;

    // ------------------------------------------------------------------
    // 1) Construire le tour des STATIONS ACTIVES dans l'ordre de ind.ids
    // ------------------------------------------------------------------
    std::vector<int> active_ids;
    active_ids.reserve(n);

    for (int pos = 0; pos < n; ++pos)
    {
        int id = ind.ids[pos];
        if (ind.mask[id - 1])   // station active ?
            active_ids.push_back(id);
    }

    const int m = static_cast<int>(active_ids.size());
    if (m <= 3) return; // trop peu de stations actives pour faire un vrai 2-opt

    // Option : s'assurer que le dépôt (id = 1) est en première position
    // Cela permet de garder une "référence" stable, comme dans le reste de ton code.
    {
        int depot_pos = -1;
        for (int i = 0; i < m; ++i)
        {
            if (active_ids[i] == 1)
            {
                depot_pos = i;
                break;
            }
        }
        if (depot_pos > 0)
        {
            // Rotation du tour de façon à avoir 1 au début
            std::rotate(active_ids.begin(),
                active_ids.begin() + depot_pos,
                active_ids.end());
        }
    }

    // ------------------------------------------------------------------
    // 2) Coût initial de l'anneau actif (uniquement la partie "ring")
    // ------------------------------------------------------------------
    double ring_cost = RingCostActiveTour(alpha, active_ids, dist);

    bool improved = true;

    // ------------------------------------------------------------------
    // 3) Boucle 2-opt avec formule Δ-coût locale
    //
    //    Complexité :
    //      - chaque passe parcourt toutes les paires (i, j) -> O(m^2)
    //      - le calcul de variation de coût est en O(1)
    //      - au lieu de recalculer tout le tour pour chaque candidat
    // ------------------------------------------------------------------
    while (improved)
    {
        improved = false;
        double best_delta = 0.0;
        int best_i = -1;
        int best_j = -1;

        // On fixe le sommet 0 (le dépôt) pour garder une "racine" stable.
        // i et j parcourent les autres positions du tour.
        for (int i = 1; i < m - 2; ++i)
        {
            for (int j = i + 1; j < m - 1; ++j)
            {
                // On coupe les arêtes (a-b) et (c-d) et on les remplace par (a-c) et (b-d).
                int a = active_ids[i - 1];
                int b = active_ids[i];
                int c = active_ids[j];
                int d = active_ids[(j + 1) % m];

                double old_cost =
                    dist[a - 1][b - 1] +
                    dist[c - 1][d - 1];

                double new_cost =
                    dist[a - 1][c - 1] +
                    dist[b - 1][d - 1];

                double delta = Cost_station(alpha, new_cost - old_cost);

                // On cherche la meilleure amélioration (delta < 0)
                if (delta < best_delta - 1e-12)
                {
                    best_delta = delta;
                    best_i = i;
                    best_j = j;
                    improved = true;
                }
            }
        }

        // Si aucune paire (i, j) n'améliore le tour, on est au minimum local
        if (!improved) break;

        // Appliquer la meilleure inversion trouvée
        std::reverse(active_ids.begin() + best_i,
            active_ids.begin() + best_j + 1);

        ring_cost += best_delta;
    }

    // ------------------------------------------------------------------
    // 4) Réinjecter le nouvel ordre actif dans ind.ids
    //
    //    On ne touche PAS au mask :
    //      - le pattern "actif / inactif" reste le même
    //    On remplace simplement, dans ind.ids, les IDs actifs dans le
    //    nouvel ordre 'active_ids', en conservant les villes inactives
    //    exactement aux mêmes positions qu'avant.
    //
    //    => Coût hors anneau inchangé (mask identique),
    //       seul le coût des arêtes entre stations actives est amélioré.
    // ------------------------------------------------------------------
    std::vector<int> new_ids = ind.ids;
    int idx_active = 0;

    for (int pos = 0; pos < n; ++pos)
    {
        int id = ind.ids[pos];
        if (ind.mask[id - 1])   // si cette position correspond à une station active
        {
            new_ids[pos] = active_ids[idx_active++];
        }
    }

    ind.ids.swap(new_ids);
}
// Insère une station donnée 'station_id' dans l'anneau ind.ids
// à la meilleure position (minimisation du surcoût sur le ring).
inline void InsertStationBestPos(
    std::vector<int>& ids,
    int station_id,
    const std::vector<std::vector<double>>& dist)
{
    const int n = static_cast<int>(ids.size());
    if (n <= 1) return;

    // 1) Trouver la position actuelle de station_id dans ids
    int old_pos = -1;
    for (int i = 0; i < n; ++i)
    {
        if (ids[i] == station_id)
        {
            old_pos = i;
            break;
        }
    }
    if (old_pos == -1) return; // sécurité : ne devrait pas arriver

    // 2) Choisir la meilleure position d'insertion (on laisse pos=0 pour le dépôt)
    int best_pos = 1;
    double best_delta = 1e18;

    for (int pos = 1; pos < n; ++pos)
    {
        int prev = ids[pos - 1];
        int next = ids[pos]; // on peut aussi considérer le tour fermé, mais ton ring
        // est ensuite retravaillé par TwoOptImproveAlpha.

        double old_cost =
            dist[prev - 1][next - 1];

        double new_cost =
            dist[prev - 1][station_id - 1] +
            dist[station_id - 1][next - 1];

        double delta = new_cost - old_cost;
        if (delta < best_delta)
        {
            best_delta = delta;
            best_pos = pos;
        }
    }

    // 3) Déplacer station_id de old_pos vers best_pos en conservant les autres
    if (old_pos < best_pos)
    {
        std::rotate(ids.begin() + old_pos,
            ids.begin() + old_pos + 1,
            ids.begin() + best_pos + 1);
    }
    else if (old_pos > best_pos)
    {
        std::rotate(ids.begin() + best_pos,
            ids.begin() + old_pos,
            ids.begin() + old_pos + 1);
    }
}

// Hill climber 1-flip sur le mask d'un individu.
//
// À chaque itération :
//   - on essaie de basculer une station (ON -> OFF ou OFF -> ON)
//   - si ON -> ON : rien
//   - si OFF -> ON : on insère la station dans le ring via InsertStationBestPos
//   - on réoptimise le ring (TwoOptImproveAlpha)
//   - on garde UNIQUEMENT le flip qui améliore le plus le coût
// On recommence tant qu'il y a une amélioration.
void ImproveMaskLocal(
    Individual& ind,
    int alpha,
    int max_station_id,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking)
{
    const int N = max_station_id;

    while (true)
    {
        // Coût actuel
        double base_cost = Total_Cost_Individual(
            alpha,
            ind.ids,
            ind.mask,
            dist,
            ranking);

        double best_delta = 0.0;
        int    best_s = -1;
        bool   best_state = false; // futur état de la station

        // Compter le nombre de stations actives
        int active_count = 0;
        for (bool b : ind.mask) if (b) ++active_count;

        // On parcourt toutes les stations sauf le dépôt
        for (int s = 2; s <= N; ++s)
        {
            int idx = s - 1;
            bool curr = ind.mask[idx];

            // Option de sécurité : garder au moins 3 actives
            if (curr && active_count <= 3)
                continue;

            Individual cand = ind;
            bool new_state = !curr;
            cand.mask[idx] = new_state;

            if (new_state)
            {
                // OFF -> ON : activer s et l'insérer correctement dans le ring
                InsertStationBestPos(
                    cand.ids,
                    s,
                    dist);
            }
            // ON -> OFF : pas besoin de toucher cand.ids ; le ring ignore les inactives.

            // Réoptimiser le ring avec 2-opt
            TwoOptImproveAlpha(
                cand,
                alpha,
                dist,
                ranking);

            double new_cost = Total_Cost_Individual(
                alpha,
                cand.ids,
                cand.mask,
                dist,
                ranking);

            double delta = new_cost - base_cost;
            if (delta < best_delta - 1e-9)
            {
                best_delta = delta;
                best_s = s;
                best_state = new_state;
            }
        }

        // Aucun flip améliorant -> minimum local
        if (best_s == -1)
            break;

        // Appliquer le meilleur flip trouvé sur ind
        int idx = best_s - 1;
        bool old_state = ind.mask[idx];
        ind.mask[idx] = best_state;

        // Si on vient d'activer la station, l'insérer dans le ring
        if (!old_state && best_state)
        {
            InsertStationBestPos(
                ind.ids,
                best_s,
                dist);
        }

        // Réoptimiser définitivement le ring pour ce mask
        TwoOptImproveAlpha(
            ind,
            alpha,
            dist,
            ranking);

        // Et la boucle while recommence avec ce nouvel individu
    }
}




// ---------------------------
// Mask crossover (genetic on/off pattern)
// ---------------------------
inline std::vector<bool> Mask_Crossover(const std::vector<bool>& m1,
    const std::vector<bool>& m2,
    int min_active = 2)
{
    int n = static_cast<int>(m1.size());
    if (m2.size() != m1.size()) {
        return m1; // fall back, should not happen
    }

    std::vector<bool> child(n, false);

    // 1) Uniform-style crossover:
    //    - if both parents agree -> keep that value
    //    - if they disagree     -> randomly choose 0 or 1
    for (int i = 0; i < n; ++i)
    {
        if (m1[i] == m2[i]) {
            child[i] = m1[i];
        }
        else {
            // one parent active, one not
            child[i] = (RandDouble() < 0.5);
        }
    }

    // 2) Force depot (id = 1 => index 0) to be active
    if (n > 0) {
        child[0] = true;
    }

    // 3) Ensure at least min_active active stations
    int active = 0;
    for (bool b : child) if (b) ++active;

    if (active < min_active)
    {
        // Prefer to activate positions that are active in at least one parent
        std::vector<int> candidates;
        candidates.reserve(n);

        for (int i = 1; i < n; ++i) // start at 1, keep depot separate
        {
            if (m1[i] || m2[i]) {
                candidates.push_back(i);
            }
        }

        // Worst case: if no candidate, allow any index >=1
        if (candidates.empty())
        {
            for (int i = 1; i < n; ++i)
                candidates.push_back(i);
        }

        while (active < min_active && !candidates.empty())
        {
            int idx = RandInt(0, static_cast<int>(candidates.size()) - 1);
            int pos = candidates[idx];
            if (!child[pos]) {
                child[pos] = true;
                ++active;
            }
            // remove this candidate
            candidates[idx] = candidates.back();
            candidates.pop_back();
        }
    }

    return child;
}
inline int PickParentIndex(const std::vector<int>& mating_pool, int mating_pool_size)
{
    // pick in [0, mating_pool_size - 1]
    int pos = RandInt(0, mating_pool_size - 1);
    return mating_pool[pos];
}
inline void EvolveSpecie(std::vector<Individual>& specie,
    const std::vector<std::vector<double>>& dist,
    const std::vector<std::vector<int>>& ranking,
    const std::vector<Node>& nodes,
    int alpha,
    double mutation_rate,
    int max_station_id,
    int elitism_count = 1,
    int DEL_PCT_RDM = 10,
	int DEL_PCT_CENT = 10,
    int SWAP_PCT = 10,
    const int INV_PCT = 10,
    const int SCR_PCT = 10,
    const int INSERTSWAP_PCT = 10,
    int max_mating_pool_size = 0.5
    )
{
    int popsize = static_cast<int>(specie.size());
    if (popsize <= 1) return;

    TimePoint start_time = Clock::now();
    // --- 1) Compute costs for the whole specie ---
    std::vector<double> costs = Total_Cost_Specie(alpha,
        specie,
        dist,
        ranking);
    auto end_time = Clock::now(); // Capture the end time
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    // Print the result
    std::cout << "Code block executed [Total_Cost_Specie] " << duration_ms.count() << " ms\n";
    // --- 2) Sort indices by increasing cost (best first) ---
    start_time = Clock::now();
    std::vector<std::pair<double, int>> cost_index(popsize);

    for (int i = 0; i < popsize; i++)
        cost_index[i] = { costs[i], i };  // (cost, index)

    std::sort(cost_index.begin(), cost_index.end());  // sorts by cost automatically

    // Extract sorted indices
    std::vector<int> indices(popsize);
    for (int i = 0; i < popsize; i++)
        indices[i] = cost_index[i].second;


    // --- 3) Build new population container ---
    std::vector<Individual> new_pop;
    new_pop.reserve(popsize);

	
    // (a) Elitism: copy the best 'elitism_count' individuals as-is
    int elite = std::min(elitism_count, popsize);
    for (int e = 0; e < elite; ++e)
    {
        new_pop.push_back(specie[indices[e]]);
    }

    // (b) Build mating pool = the best 'mating_pool_size' individuals
    int mating_pool_size = std::max(2, static_cast<int>(max_mating_pool_size * popsize));
    if (mating_pool_size > popsize) mating_pool_size = popsize;

    std::vector<int> mating_pool(mating_pool_size);
    for (int i = 0; i < mating_pool_size; ++i)
    {
        mating_pool[i] = indices[i]; // indices of best individuals
    }

    
    end_time = Clock::now(); // Capture the end time
    duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    // Print the result
    std::cout << "Code block executed [Before_Mating] " << duration_ms.count() << " ms\n";

    start_time = Clock::now();
    // --- 5) Fill the rest of the population ---
    while (static_cast<int>(new_pop.size()) < popsize)
    {
        // Parents chosen ONLY among the best 'mating_pool_size' individuals
        int i1 = PickParentIndex(mating_pool, mating_pool_size);
        int i2 = PickParentIndex(mating_pool, mating_pool_size);

        const Individual& p1 = specie[i1];
        const Individual& p2 = specie[i2];

        // Crossover sur la permutation (ids)
        std::vector<int> child_ids = Order_Crossover(p1.ids, p2.ids);

        // Crossover génétique sur le mask
        std::vector<bool> child_mask =
            Mask_Crossover(p1.mask, p2.mask, /*min_active*/ 2);

        Individual child;
		child.ids = std::move(child_ids); // basically change the pointer at the place of copying. Apparently faster that's waht tehy said.
        child.mask = std::move(child_mask);

        // Mutation decision:
        bool force_mutation = (child.ids == p1.ids) || (child.ids == p2.ids);

        if (force_mutation || RandDouble() < mutation_rate)
        {
            child = Mutations(DEL_PCT_RDM,
                DEL_PCT_CENT,
                SWAP_PCT,
                INV_PCT,
                SCR_PCT,
                INSERTSWAP_PCT,
                child,
                max_station_id,
                nodes,
                dist);
        }

        // If you want purely genetic GA, keep 2-opt commented out:
        // TwoOptImproveAlpha(child, alpha, dist, ranking);
        TwoOptImproveAlpha(child, alpha, dist, ranking);

        new_pop.push_back(std::move(child));
    }

    end_time = Clock::now(); // Capture the end time
    duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    // Print the result
    std::cout << "Code block executed [After_Mating] " << duration_ms.count() << " ms\n";

    start_time = Clock::now();
    // --- 5.5) Optional: local search on mask for the best of this specie ---
    int best_idx = 0;
    double best_cost = 1e18;
    for (int i = 0; i < (int)new_pop.size(); ++i)
    {
        double c = Total_Cost_Individual(alpha,
            new_pop[i].ids,
            new_pop[i].mask,
            dist,
            ranking);
        if (c < best_cost)
        {
            best_cost = c;
            best_idx = i;
        }
    }
    end_time = Clock::now(); // Capture the end time
    duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    // Print the result
    std::cout << "Code block executed [After_Local_Search] " << duration_ms.count() << " ms\n";
    // Hill climber sur le mask du meilleur de l'espèce
    start_time = Clock::now();

    ImproveMaskLocal(new_pop[best_idx],
        alpha,
        max_station_id,
        dist,
        ranking);
    // --- 6) Replace old specie by the new one ---
    specie.swap(new_pop);
    end_time = Clock::now(); // Capture the end time
    duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    // Print the result
    std::cout << "Code block executed [After_Improve_Local_Mask] " << duration_ms.count() << " ms\n";
}



