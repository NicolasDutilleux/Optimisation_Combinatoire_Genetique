# Ring Optimization - Genetic Algorithm

## Overview

This project implements a **genetic algorithm** to solve a ring network optimization problem. The goal is to find the optimal subset of stations to include in a ring network, minimizing total connection costs.

## Problem Description

Given N stations with coordinates, we want to:
1. Select a subset of stations to form a **ring** (closed loop)
2. Connect remaining stations to their **nearest ring station**
3. Minimize total cost = (ring edges cost) + (out-of-ring connections cost)

The cost formula uses a parameter **alpha** (default=3):
- Ring edge cost: `alpha * distance`
- Out-of-ring connection: `(10 - alpha) * distance`

---

## File Structure

```
??? main.c                      # Main program with thread pool
??? core/
?   ??? Node.h/.c               # Station data structure (id, x, y)
?   ??? Individual.h/.c         # Solution representation (ring + cached cost)
??? generation/
?   ??? PopulationInit.h/.c     # Random initial population generator
??? evolution/
?   ??? EvolveSpecie.h/.c       # One generation of genetic evolution
??? genetic/
?   ??? Selection.h/.c          # Tournament selection
?   ??? Crossover.h/.c          # Slice crossover operator
?   ??? Mutation.h/.c           # Add/remove/swap/invert/scramble mutations
??? local_search/
?   ??? TwoOpt.h/.c             # 2-opt local improvement
??? cost/
?   ??? Cost.h/.c               # Fitness evaluation functions
??? utils/
?   ??? Distance.h/.c           # Distance matrix computation
?   ??? Random.h/.c             # Random number generator
?   ??? FileIO.h/.c             # Dataset loading
?   ??? main_helpers.h/.c       # Argument parsing, progress reporting
??? data/
    ??? 51/51_data.txt          # Example dataset (51 stations)
```

---

## Core Components

### 1. `main.c` - Main Program

**Purpose**: Entry point, orchestrates the entire algorithm.

**Key Features**:
- **Thread Pool**: Uses Windows semaphore-based thread pool for parallel evolution
- **Command Line Args**: `-g` generations, `-s` species, `-p` population, `-t` threads, `--logs`, `--timers`
- **Progress Reporting**: Shows best cost every N generations

**Flow**:
1. Load dataset
2. Compute distance matrix and ranking
3. Initialize random population
4. Create thread pool
5. Evolution loop (parallel)
6. Report final results

### 2. `core/Individual.h/.c` - Solution Representation

```c
typedef struct {
    int* active_ring;    // Array of station IDs in the ring
    int ring_size;       // Number of stations in ring
    int ring_capacity;   // Allocated size
    double cached_cost;  // Cached fitness value
} Individual;
```

**Functions**:
- `Individual_Init()` - Allocate memory
- `Individual_Free()` - Free memory
- `Individual_Copy()` - Deep copy

### 3. `evolution/EvolveSpecie.c` - Genetic Evolution

**One generation consists of**:

1. **Evaluate**: Compute cost for all individuals (uses cache)
2. **Sort**: Rank by fitness (lowest cost = best)
3. **Elite Selection**: Keep best N individuals unchanged
4. **Offspring Generation**:
   - Select 2 parents from mating pool (top 50%)
   - Apply **crossover** (slice crossover)
   - Apply **mutation** (probabilistic)
   - Apply **2-opt** local search
   - Compute fitness
5. **Replace**: Swap old population with new

### 4. `genetic/Crossover.c` - Slice Crossover

Takes a random slice from Parent A, fills rest from Parent B (avoiding duplicates).

```
Parent A: [1, 2, 3, 4, 5]
Parent B: [3, 5, 1, 4, 2]
Cut: [1, 3]
Child:    [2, 3, 4] + [5, 1] = [2, 3, 4, 5, 1]
```

### 5. `genetic/Mutation.c` - Mutation Operators

| Operator | Description |
|----------|-------------|
| Add Node | Insert best-fit station not in ring |
| Remove Node | Remove random station (min size = 3) |
| Swap | Exchange two random positions |
| Inversion | Reverse a random segment |
| Scramble | Shuffle a random segment |

Each has a probability (default: add=15%, remove=10%, swap=15%, inv=5%, scramble=5%).

### 6. `local_search/TwoOpt.c` - 2-Opt Improvement

Classic 2-opt for TSP: tries all edge swaps, applies best improvement, repeats until no improvement.

```
Before: A-B ... C-D
After:  A-C ... B-D (segment B..C reversed)
```

### 7. `cost/Cost.c` - Fitness Evaluation

```c
Total_Cost = RingCost + OutOfRingCost

RingCost = sum of (alpha * dist[i][i+1]) for all ring edges
OutOfRingCost = sum of ((10-alpha) * dist[s][nearest]) for all non-ring stations
```

**Optimization**: Uses stack allocation for arrays ?512 elements to avoid malloc overhead.

---

## Thread Pool Architecture

```
Main Thread                    Worker Threads (N)
     ?                              ?
     ?  ???????????????????????     ?
     ?  ? Build task array    ?     ?
     ?  ???????????????????????     ?
     ?           ?                  ?
     ?  ReleaseSemaphore(N)  ???????? WaitForSingleObject(semaphore)
     ?           ?                  ?
     ?  WaitForSingleObject  ???????? InterlockedIncrement(task_idx)
     ?  (done_event)                ? Execute EvolveSpecie()
     ?           ?                  ? SetEvent(done) when all complete
     ?           ?                  ?
     ?  Next generation...          ?
```

**Why Semaphore?** Unlike manual-reset events, semaphore releases exactly N tokens, ensuring each task is picked up by exactly one worker.

---

## Usage

```bash
# Default: 100 species, 50 pop, auto threads
program.exe

# Custom parameters
program.exe -g 5000 -s 50 -p 30 -t 4

# With timing info
program.exe --timers

# With detailed logs
program.exe --logs

# All options
program.exe -g 1000 -s 100 -p 50 -t 8 -l 25 --timers --logs
```

**Arguments**:
| Flag | Description | Default |
|------|-------------|---------|
| `-g N` | Max generations | 10000 |
| `-s N` | Number of species | 100 |
| `-p N` | Population per species | 50 |
| `-t N` | Number of threads | auto |
| `-l N` | Log interval | 50 |
| `--logs` | Enable detailed logs | OFF |
| `--timers` | Enable timing info | OFF |
| `-v` | Verbose mode | OFF |

---

## Performance Notes

1. **Stack Allocation**: Cost.c, Crossover.c, Mutation.c use stack buffers for small arrays (?256 elements) to avoid malloc/free overhead in hot paths.

2. **Cost Caching**: Each individual caches its cost. Only recomputed after mutation.

3. **Thread Pool**: Species evolve in parallel. With 4 threads and 100 species, each thread handles ~25 species per generation.

4. **2-Opt Limit**: Limited to 10 iterations to balance quality vs speed.

---

## Expected Output

```
============================================
  GENETIC ALGORITHM - RING OPTIMIZATION
============================================

[CONFIG]
  Generations: 10000
  Species:     100
  Population:  50 per species
  Threads:     4

[STEP 1] Loading dataset...
         51 stations loaded

[STEP 2] Computing distance matrix...
         Done (0.1 ms)

...

  [GENERATION 50]
  Best: cost=1298.53 (species 33, individual 1)
  >>> IMPROVEMENT: 2786.22 -> 1298.53

...

  BEST SOLUTION
  -------------
  Cost:    1298.53
  Ring:    1 51 37 49 39 10 33 45 19 41 13 25 23 26 3 ...
  Size:    18 nodes

  TOTAL TIME: 45.2 seconds
```

---

## Algorithm Parameters

| Parameter | Value | Description |
|-----------|-------|-------------|
| Alpha | 3 | Cost weighting (ring vs out-of-ring) |
| Mutation Rate | 0.25 | Probability of mutation |
| Elitism | 0 | Elite individuals (0 = none) |
| Mating Pool | 0.5 | Top 50% can reproduce |
| Stagnation Limit | 50 | Stop if no improvement |

---

## Building

Compile with any C compiler supporting C99+:

```bash
# GCC
gcc -O2 -o ring_ga main.c core/*.c generation/*.c evolution/*.c genetic/*.c local_search/*.c cost/*.c utils/*.c -lkernel32

# MSVC
cl /O2 main.c core/*.c generation/*.c evolution/*.c genetic/*.c local_search/*.c cost/*.c utils/*.c
```

---

## License

MIT License
