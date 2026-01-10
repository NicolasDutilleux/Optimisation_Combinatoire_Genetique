// utils/Visualize.h - Ring visualization
#ifndef VISUALIZE_H
#define VISUALIZE_H

#include "core\Node.h"
#include "core\Individual.h"

// Generate an HTML file with SVG visualization of the ring
// Opens the file in the default browser
void Visualize_Ring(const Individual* ind, const Node* nodes, int total_stations,
                    double cost, int alpha, int generation);

#endif
