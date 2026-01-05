#ifndef MAIN_HELPERS_H
#define MAIN_HELPERS_H

#include <stdio.h>
#include <stdlib.h>

#include "core\Individual.h"
#include "core\Node.h"

// Small helper used for reporting sorted species
typedef struct { double cost; int species_id; int best_idx; } SpeciesCostLocal;

// Print current timestamp with a label
void print_now(const char* label);

// parse command-line arguments (updates provided variables)
void parse_args(int argc, char** argv, int* max_generations, int* LOG_INTERVAL, int* NUM_SPECIES, int* POP_SIZE, int* requested_workers, int* verbose);

// perform detailed evaluation and reporting (serial implementation)
void evaluate_and_report(int gen, int NUM_SPECIES, int POP_SIZE, double** dist, int** dist_ranking, Node* node_vector, int total_stations, int ALPHA, Individual** species, double* old_best_ptr, int* stagnation_count_ptr, double* MUTATION_RATE_ptr);

#endif // MAIN_HELPERS_H
