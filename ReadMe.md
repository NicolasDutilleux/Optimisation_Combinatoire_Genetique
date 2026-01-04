# Ring-Station Genetic Algorithm

This project implements a genetic algorithm with local search to solve a **ring + assignment** problem:

- We have a set of stations (points with coordinates).
- We must choose:
  - a subset of stations that form a **ring** (a cycle),
  - and connect every non-ring station to its **nearest ring station**.
- The total cost is a weighted sum of:
  - the length of the ring,
  - the assignment distances from non-ring stations to their nearest ring station.

The code is written in C++ (C++17 compatible) and works on datasets similar to TSPLIB (ID + X + Y).

---

## Problem Definition

Given:

- A set of stations \( V = \{1, 2, \dots, N\} \) with coordinates \((x_i, y_i)\),
- A parameter \( \alpha \in [0, 10] \) controlling the balance between ring cost and assignment cost.

We want to find:

- A permutation of all stations (order in which they could appear on a tour).
- A Boolean **mask** indicating which stations are **on the ring** and which are **off the ring**.

The cost of an individual solution is:

- **Ring cost**  
  Sum of distances along the cycle formed by the active stations (mask = 1).

- **Out-of-ring cost**  
  For every inactive station, distance to the **nearest active station**, multiplied by a weight depending on \( \alpha \).

Total cost:

\[
\text{TotalCost} = \alpha \cdot \text{RingDistance} + (10 - \alpha) \cdot \text{AssignmentDistance}
\]

---

## Data Format

Input dataset example (e.g. `Datasets/51/51_data.txt`):

```text
DIMENSION : 51
BEGIN
1  x1  y1
2  x2  y2
...
N  xN  yN

First line: dimension info (ignored after reading).

Second line: a keyword (e.g. BEGIN) also ignored.

Then one line per station: id x y.

## High-Level Algorithm

The program runs the following pipeline:

Load data

readDataset() reads the set of nodes from the dataset file.

Each node is stored as a Node { id, x, y }.

Precompute geometry

Compute_Distances_2DVector() builds a full distance matrix dist[i][j].

Distance_Ranking_2DVector() builds, for each node, a sorted list of neighbors by distance.
This is used to quickly find nearest active stations.

Initialize populations

We maintain NUM_SPECIES independent species.

For each species, Random_Generation() creates POP_SIZE individuals.

Each Individual contains:

ids: a permutation of [1..N] (depot 1 is kept at the front in the initial shuffle),

mask: Boolean vector of size N:

mask[0] (station 1) is always true (always on the ring),

a random number of other positions are set to true (between 3 and N active stations).

Evaluate cost

RingCostOnly() computes the ring cost given ids and mask.

OutRingCostOnly() computes the assignment cost using the precomputed neighbor rankings.

Total_Cost_Individual() combines both.

Main evolutionary loop
For gen = 0 .. MAX_GENERATIONS:

For each species:

Call EvolveSpecie() to create the next generation.

Every MAX_GENERATIONS / 100 generations:

Compute the best individual across all species:

Total_Cost_Specie() to get costs per species,

Select_Best() to find the minimum.

Print the best cost and the corresponding permutation + mask.

Export a visualization with PlotIndividualSVG().

Optionally, adapt some mutation parameters when progress stalls
(deletion percentages, swap percentage, mutation rate).

Inside EvolveSpecie()

For one species (one population of individuals):

Evaluate

Compute costs of all individuals with Total_Cost_Specie().

Sort & elitism

Sort indices by increasing cost.

Copy the top elitism_count individuals directly into the new population.

Mating pool

Select the best fraction of the population (e.g. top 50%) as the mating pool.

Parents will be randomly drawn from this pool.

Reproduction
While new_pop.size() < popsize:

Parent selection

Choose two parents from the mating pool at random.

Crossover

Order_Crossover(parentA.ids, parentB.ids)
→ classical OX crossover on permutations, preserving city set and relative ordering.

Mask_Crossover(parentA.mask, parentB.mask)
→ uniform crossover on the mask, with:

depot (index 0) forced active,

enforcing a minimum number of active stations.

Mutation

With probability mutation_rate or if child.ids is identical to one parent:

Mutation_Deletion_Centroid()
→ deactivate one active station farthest from the centroid.

Mutation_RandomMultiDeletion()
→ randomly deactivate several active stations (up to max_del).

Permutation mutations (on ids):

Mutation_Swap_Simple() (swap two positions),

Mutation_Inversion() (reverse a segment),

Mutation_Scramble() (shuffle a segment),

Mutation_Insert_Swap() (remove & reinsert a station elsewhere).

Mutation_Insertion_Heuristic()
→ possibly insert a new inactive station into the ring at the best local position.

Local search (2-opt on the active ring)

TwoOptImproveAlpha(child, alpha, dist, ranking):

extracts the tour of active stations,

performs a 2-opt local search (edge swaps) on the ring only,

reinjects the improved order back into child.ids without changing the mask.

Add child

Push the mutated + locally improved child into new_pop.

Replacement

Replace the old population with specie.swap(new_pop).

Visualization

PlotIndividualSVG() draws, for the best individual at some generation:

All stations as brown dots.

Active ring stations as red dots, Depot 1 as a blue dot.

The ring edges as red lines forming a closed loop.

The SVG is written to Images/plot_individual_<generation>.svg.

This allows you to see how the ring evolves over generations.

File Structure

Indicative file layout:

main.cpp
Entry point. Sets parameters, loads data, launches the GA loop and visualization.

logic_function.h / .cpp
Core algorithm:

distance computation,

cost functions,

random generation,

mutations, crossover,

2-opt improvement,

species evolution.

hierarchy_and_print_utils.h / .cpp
Utilities:

dataset reading (readDataset),

console printing helpers,

SVG plotting.

Node.h
Definition of Node { id, x, y }.

Individual.h
Definition of Individual { std::vector<int> ids; std::vector<bool> mask; }.

Datasets/
Problem instances (e.g. 51_data.txt).

Images/
Generated SVG plots of intermediate/best solutions.

Build & Run

Example using g++ (adapt files as needed):

g++ -std=c++17 -O3 \
    main.cpp \
    logic_function.cpp \
    hierarchy_and_print_utils.cpp \
    -o ring_ga

./ring_ga


You can tune the main parameters directly in main.cpp:

NUM_SPECIES

POP_SIZE

MAX_GENERATIONS

ALPHA (trade-off between ring and assignment cost)

MUTATION_RATE

DEL_PCT_RDM, DEL_PCT_CENT, SWAP_PCT, INV_PCT, SCR_PCT, INSERTSWAP_PCT.

Possible Extensions

Ideas to go further:

Add a hill climber specifically on the mask (activation/deactivation of stations).

Introduce a simulated annealing acceptance rule to escape local minima.

Add tabu search on mask changes.

Log ring cost vs. out-of-ring cost separately for better analysis.

Generalize dataset loading and allow selecting instances from the command line.