#include "utils\main_helpers.h"
#include <string.h>
#include <time.h>
#include <windows.h>
#include <process.h>

#include "utils\hierarchy_and_print_utils.h"
#include "utils\FileIO.h"
#include "utils\Distance.h"
#include "utils\Random.h"
#include "evolution\EvolveSpecie.h"
#include "cost\Cost.h"
#include "genetic\Selection.h"

static int cmp_sc(const void* a, const void* b)
{
    double da = ((const SpeciesCostLocal*)a)->cost;
    double db = ((const SpeciesCostLocal*)b)->cost;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

void print_now(const char* label)
{
    time_t t = time(NULL);
    struct tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    printf("[%s] %s\n", buf, label);
}

void parse_args(int argc, char** argv, int* max_generations, int* LOG_INTERVAL, int* NUM_SPECIES, int* POP_SIZE, int* requested_workers, int* verbose)
{
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-v") == 0) { *verbose = 1; }
        else if (strcmp(argv[i], "-g") == 0 && i + 1 < argc) { *max_generations = atoi(argv[++i]); }
        else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) { *NUM_SPECIES = atoi(argv[++i]); }
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) { *POP_SIZE = atoi(argv[++i]); }
        else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) { *LOG_INTERVAL = atoi(argv[++i]); }
        else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) { *requested_workers = atoi(argv[++i]); if (*requested_workers < 0) *requested_workers = 0; }
        else if (argv[i][0] >= '0' && argv[i][0] <= '9') { *max_generations = atoi(argv[i]); }
    }
}


void evaluate_and_report(int gen, int NUM_SPECIES, int POP_SIZE, double** dist, int** dist_ranking, Node* node_vector, int total_stations, int ALPHA, Individual** species, double* old_best_ptr, int* stagnation_count_ptr, double* MUTATION_RATE_ptr)
{
    // serial, straightforward evaluation so it's easy to follow
    SpeciesCostLocal* species_costs = (SpeciesCostLocal*)malloc(NUM_SPECIES * sizeof(SpeciesCostLocal));
    if (!species_costs) return;

    for (int s = 0; s < NUM_SPECIES; ++s) {
        double* costs = Total_Cost_Specie(ALPHA, species[s], POP_SIZE, total_stations, (const double**)dist, (const int**)dist_ranking);
        if (!costs) {
            species_costs[s].cost = 1e18;
            species_costs[s].best_idx = -1;
        } else {
            int best_idx = Select_Best(costs, POP_SIZE);
            species_costs[s].cost = costs[best_idx];
            species_costs[s].best_idx = best_idx;
            free(costs);
        }
        species_costs[s].species_id = s;
    }

    qsort(species_costs, NUM_SPECIES, sizeof(SpeciesCostLocal), cmp_sc);

    printf("\n[GEN %d] Progress Report\n", gen);
    printf("  [TOP 5 SPECIES]\n");
    int top = (5 < NUM_SPECIES) ? 5 : NUM_SPECIES;
    for (int i = 0; i < top; ++i) {
        printf("    #%d - Species %3d: %.2f\n", i + 1, species_costs[i].species_id, species_costs[i].cost);
    }

    printf("\n  [GLOBAL BEST] Cost: %.2f\n", species_costs[0].cost);

    int best_spec = species_costs[0].species_id;
    int best_idx = species_costs[0].best_idx;
    if (best_idx >= 0) {
        printf("  [BEST INDIVIDUAL] species=%d idx=%d ring_size=%d: ", best_spec, best_idx, species[best_spec][best_idx].ring_size);
        for (int kk = 0; kk < species[best_spec][best_idx].ring_size; ++kk) {
            printf("%d ", species[best_spec][best_idx].active_ring[kk]);
        }
        printf("\n");
    }

    if (species_costs[0].cost < *old_best_ptr && best_idx >= 0) {
        int assign_len = 0;
        int* assign_pairs = BuildAssignmentPairs(species[best_spec][best_idx].active_ring, species[best_spec][best_idx].ring_size, (const int**)dist_ranking, total_stations, &assign_len);
        PlotIndividualSVG_C(species[best_spec][best_idx].active_ring, species[best_spec][best_idx].ring_size, node_vector, total_stations, gen, assign_pairs, assign_len);
        if (assign_pairs) free(assign_pairs);
    }

    if (species_costs[0].cost < *old_best_ptr) {
        *old_best_ptr = species_costs[0].cost;
        *stagnation_count_ptr = 0;
        printf("  [IMPROVEMENT] New best found\n\n");
    } else {
        *stagnation_count_ptr += 1;
        if ((*stagnation_count_ptr) % 5 == 0) {
            double new_rate = (*MUTATION_RATE_ptr) * 1.25;
            if (new_rate > 0.5) new_rate = 0.5;
            if (new_rate > *MUTATION_RATE_ptr) {
                *MUTATION_RATE_ptr = new_rate;
                printf("  [MUTATION] Increased mutation rate to %.4f due to stagnation\n", *MUTATION_RATE_ptr);
            }
        }
        printf("  [NO IMPROVEMENT] stagnation_count=%d\n\n", *stagnation_count_ptr);
    }

    free(species_costs);
}
