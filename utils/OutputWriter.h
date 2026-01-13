// utils/OutputWriter.h
// Write final solution to text file
//
// Creates "Genetic_Solution.txt" with:
//   - Problem parameters (dimension, alpha)
//   - Best cost found
//   - Ring nodes (tour order)
//   - Assignment of non-ring nodes to nearest ring node

#ifndef OUTPUT_WRITER_H
#define OUTPUT_WRITER_H

#include "core\Individual.h"
#include "core\Node.h"

// Write complete solution to "Genetic_Solution.txt"
//
// Parameters:
//   best_individual  - Best solution found
//   total_stations   - Total number of stations
//   alpha            - Alpha parameter
//   best_cost        - Cost of best solution
//   dist_ranking     - Distance ranking matrix (for assignments)
//
void WriteSolutionToFile(
    const Individual* best_individual,
    int total_stations,
    int alpha,
    double best_cost,
    const int** dist_ranking
);

#endif // OUTPUT_WRITER_H
