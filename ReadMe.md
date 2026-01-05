# Ring-Station Genetic Algorithm Optimizer

A high-performance genetic algorithm for solving the Ring Scheduling Problem (RSP).

## Problem Definition

Partition N stations into:
- **Active Ring**: Closed tour through selected stations  
- **Inactive Stations**: Assigned to nearest ring station

Minimize: `Cost = ?·RingCost + (10-?)·AssignmentCost`

Where:
- **? ? [0,10]**: Trade-off parameter (higher = favor smaller rings)
- **RingCost**: Total distance of the ring tour
- **AssignmentCost**: Sum of distances from inactive stations to nearest ring

## Algorithm Overview

### Genetic Operators
- **Crossover**: Slice-based recombination (combines parent segments)
- **Mutations**: Add/Remove nodes, Swap positions, Inversion, Scramble
- **Selection**: Tournament selection with elitism
- **Local Search**: 2-opt edge swapping for tour improvement

### Representation
```cpp
struct Individual {
    std::vector<int> active_ring;  // Variable-length ring of station IDs
};
```

No mask needed - ring directly represents active stations.

## Building & Running

### Compile
```bash
# Visual Studio
msbuild OptimisationCombinatoire.sln /p:Configuration=Release

# GCC/Clang
g++ -O3 -std=c++17 main.cpp genetic/*.cpp evolution/*.cpp cost/*.cpp \
    utils/*.cpp local_search/*.cpp generation/*.cpp -o rsp_optimizer
```

### Run
```bash
./rsp_optimizer
```

## Configuration

Edit parameters in `main.cpp`:

```cpp
const int NUM_SPECIES = 3;           // Number of species
const int POP_SIZE = 100;            // Individuals per species
const int MAX_GENERATIONS = 10000;   // Generations (with early stopping)
const int ALPHA = 3;                 // Cost parameter

double MUTATION_RATE = 0.20;         // Mutation probability
const int ELITISM = 3;               // Best individuals to preserve

int ADD_PCT = 20;                    // Add node mutation rate
int REMOVE_PCT = 15;                 // Remove node mutation rate
int SWAP_PCT = 15;                   // Swap mutation rate
```

## Performance Characteristics

- **Pre-computed distance matrix**: O(1) distance lookups
- **Limited 2-opt**: Only first 1/3 of ring, max 20 iterations
- **Hash-based membership**: O(1) checks instead of O(n)
- **Early stopping**: Stops if no improvement after 1000 generations

### Expected Performance
- **51 nodes**: ~5-10 seconds to convergence
- **225 nodes**: ~2-5 minutes to convergence
- **Improvement**: 30-50% faster than naive approaches

## Project Structure

```
OptimisationCombinatoire/
??? main.cpp                          # Entry point and evolution loop
??? core/
?   ??? Individual.h                  # Individual representation
?   ??? Node.h                        # Station with (id, x, y)
??? genetic/
?   ??? Mutation.cpp/h                # 5 mutation operators
?   ??? Crossover.cpp/h               # 3 crossover methods
?   ??? Selection.cpp/h               # Selection strategies
??? evolution/
?   ??? EvolveSpecie.cpp/h            # Per-species evolution
??? local_search/
?   ??? TwoOpt.cpp/h                  # 2-opt optimization
??? cost/
?   ??? Cost.cpp/h                    # Cost computation
??? utils/
?   ??? Distance.cpp/h                # Distance matrix & ranking
?   ??? Random.cpp/h                  # PRNG wrapper
?   ??? hierarchy_and_print_utils.h   # I/O utilities
??? generation/
?   ??? PopulationInit.cpp/h          # Population initialization
??? data/
    ??? 51/51_data.txt                # Example dataset
```

## Key Optimizations

1. **Pre-computed Distance Matrix** - Compute once at startup, use in O(1)
2. **Limited 2-opt** - Check only top 1/3 of ring, max 20 iterations
3. **Hash Table Membership** - O(1) checks instead of O(n) searches
4. **Smart Node Insertion** - Add at best position, not random
5. **Reduced Logging** - Log every 500 generations (not 100)
6. **Early Stopping** - Stop if stagnated for 1000 generations

See `CHANGES.md` for detailed optimization details.

## Expected Results

For **225 nodes with ?=3**:
- **Initial solution**: ~8,000-10,000
- **After 500 gens**: ~3,000-5,000
- **Final solution**: ~2,000-3,000
- **Time**: 2-5 minutes

## License

Part of optimization study. See GitHub for details.

## Author

Nicolas Dutilleux  
GitHub: https://github.com/NicolasDutilleux/Optimisation_Combinatoire_Genetique
