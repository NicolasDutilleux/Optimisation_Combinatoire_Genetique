// utils/main_helpers.h
// Helper functions for the main program
//
// Provides:
//   - Command line argument parsing
//   - Progress reporting
//   - Time formatting

#ifndef MAIN_HELPERS_H
#define MAIN_HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include "core\Individual.h"
#include "core\Node.h"

// Structure for tracking species performance
typedef struct {
    double cost;       // Best cost in species
    int species_id;    // Species index
    int best_idx;      // Index of best individual
} SpeciesCostLocal;

// Print current timestamp with a label
// Example output: [2024-01-09 12:34:56] Starting evolution
void print_now(const char* label);

// Parse command line arguments
//
// Supported flags:
//   -g <N>      Set max generations
//   -s <N>      Set number of species
//   -p <N>      Set population size per species
//   -l <N>      Set log interval
//   -t <N>      Set number of worker threads (ignored in current version)
//   -v          Enable verbose output
//   --logs      Enable detailed logging
//   --timers    Enable timing information
//   <number>    Set max generations (positional)
//
void parse_args(
    int argc, char** argv,
    int* max_generations,
    int* LOG_INTERVAL,
    int* NUM_SPECIES,
    int* POP_SIZE,
    int* requested_workers,
    int* verbose,
    int* enable_logs,
    int* enable_timers
);

// Evaluate all species and report progress
//
// This function:
//   1. Computes cost for all individuals in all species
//   2. Finds the global best
//   3. Prints progress report
//   4. Optionally generates SVG visualization (if logs enabled)
//   5. Updates stagnation counter
//   6. Adjusts mutation rate if stagnating
//
void evaluate_and_report(
    int gen,
    int NUM_SPECIES,
    int POP_SIZE,
    double** dist,
    int** dist_ranking,
    Node* node_vector,
    int total_stations,
    int ALPHA,
    Individual** species,
    double* old_best_ptr,
    int* stagnation_count_ptr,
    double* MUTATION_RATE_ptr,
    int enable_logs,
    int enable_timers
);

#endif // MAIN_HELPERS_H
