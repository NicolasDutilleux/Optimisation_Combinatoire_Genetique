// utils/ThreadPool.h
// =============================================================================
// THREAD POOL FOR PARALLEL SPECIES EVOLUTION
// =============================================================================
//
// WHY A THREAD POOL INSTEAD OF std::thread-LIKE APPROACH?
// --------------------------------------------------------
// In C++ with std::thread, you might do:
//     std::vector<std::thread> threads;
//     for (task : tasks) threads.emplace_back(worker_func, task);
//     for (auto& t : threads) t.join();
//
// This creates/destroys threads every generation - EXPENSIVE!
// Thread creation on Windows takes ~0.5-1ms per thread.
// With 100 species × 1000 generations = 100,000 thread creates = ~50-100 seconds wasted!
//
// SOLUTION: Thread Pool (create once, reuse forever)
// - Workers are created at startup and sleep until needed
// - Each generation, we just "wake them up" with new tasks
// - Workers go back to sleep when done (not destroyed)
//
// WINDOWS THREADING vs C++ std::thread
// -------------------------------------
// C++ std::thread:     std::thread t(func, args...);  t.join();
// Windows equivalent:  HANDLE h = _beginthreadex(...); WaitForSingleObject(h, INFINITE);
//
// The main differences:
// 1. Windows uses HANDLEs (opaque pointers) instead of objects
// 2. Windows has explicit synchronization primitives (Semaphore, Event, CriticalSection)
// 3. No RAII - you must manually CloseHandle() when done
//
// SYNCHRONIZATION PRIMITIVES USED
// --------------------------------
// Semaphore: Like a counter that threads can wait on
//   - ReleaseSemaphore(sem, N) ? adds N to counter, wakes N waiting threads
//   - WaitForSingleObject(sem) ? decrements counter (blocks if counter=0)
//
// Event: A simple signal flag
//   - SetEvent(evt) ? sets flag to "signaled"
//   - ResetEvent(evt) ? clears flag
//   - WaitForSingleObject(evt) ? blocks until signaled
//
// InterlockedIncrement: Atomic counter increment (like std::atomic<int>::fetch_add)
//
// =============================================================================

#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "core\Individual.h"
#include "core\Node.h"

// =============================================================================
// EVOLUTION TASK STRUCTURE
// =============================================================================
// Each task contains everything needed to evolve one species for one generation.
// Tasks are independent - no shared mutable state between them.
typedef struct {
    Individual* specie;         // Array of individuals in this species
    int pop_size;               // Number of individuals
    double** dist;              // Distance matrix (READ-ONLY, shared)
    int** ranking;              // Distance ranking (READ-ONLY, shared)
    Node* nodes;                // Station data (READ-ONLY, shared)
    int total_stations;         // Number of stations
    int alpha;                  // Cost weighting parameter
    double mutation_rate;       // Probability of mutation
    int elitism;                // Number of elites to preserve
    int add_pct, remove_pct, swap_pct, inv_pct, scr_pct;  // Mutation percentages
} EvolveTask;

// =============================================================================
// THREAD POOL API
// =============================================================================

// Initialize the thread pool with N worker threads.
// Workers are created and immediately start waiting for tasks.
// Returns 1 on success, 0 on failure.
int ThreadPool_Init(int num_workers);

// Submit an array of tasks and BLOCK until all complete.
// This is the main entry point called each generation.
// Flow:
//   1. Store tasks in global array
//   2. Signal workers (release semaphore N times)
//   3. Workers wake up, grab tasks atomically, execute
//   4. Last worker to finish signals completion event
//   5. This function returns
void ThreadPool_Run(EvolveTask* tasks, int count, int enable_logs, int enable_timers);

// Shutdown the pool: signal workers to exit, wait for them, free resources.
// Call this once at program end.
void ThreadPool_Destroy(void);

#endif // THREAD_POOL_H
