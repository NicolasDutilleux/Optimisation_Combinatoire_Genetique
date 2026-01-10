// utils/ThreadPool.c
// =============================================================================
// THREAD POOL IMPLEMENTATION - PRODUCER/CONSUMER PATTERN
// =============================================================================
//
// ARCHITECTURE OVERVIEW
// ----------------------
//
//   Main Thread                      Worker Threads (N of them)
//   ???????????                      ??????????????????????????
//        ?                                    ?
//        ?  ThreadPool_Init()                 ?
//        ?  ?????????????????                 ?
//        ?  Create N workers ???????????????  ? (workers start, immediately
//        ?                                    ?  wait on semaphore)
//        ?                                    ?
//        ?  ThreadPool_Run(tasks)             ?
//        ?  ?????????????????????             ?
//        ?  1. Store tasks globally           ?
//        ?  2. ReleaseSemaphore(N) ?????????  ? Workers wake up!
//        ?  3. WaitForSingleObject(done) ???  ?
//        ?     (blocks here)               ?  ? Each worker:
//        ?                                 ?  ?  - InterlockedIncrement(&next_task)
//        ?                                 ?  ?  - Execute task
//        ?                                 ?  ?  - InterlockedIncrement(&done_count)
//        ?                                 ?  ?  - If last: SetEvent(done) ???
//        ?  4. Returns                        ?
//        ?                                    ? Workers go back to waiting
//        ?                                    ?
//
// WHY InterlockedIncrement FOR TASK DISTRIBUTION?
// ------------------------------------------------
// Problem: How do N workers grab M tasks without conflicts?
// 
// Bad approach (race condition):
//     int task = next_task;    // Thread A reads 0
//     next_task++;             // Thread B reads 0 (before A's increment!)
//                              // Both threads work on task 0!
//
// Good approach (atomic):
//     int task = InterlockedIncrement(&next_task) - 1;
//     // This is ONE atomic operation - no race possible
//     // Similar to: task = next_task.fetch_add(1) in C++
//
// =============================================================================

#include "ThreadPool.h"
#include "evolution\EvolveSpecie.h"

#include <windows.h>
#include <process.h>    // For _beginthreadex (safer than CreateThread for C runtime)
#include <stdlib.h>

// =============================================================================
// GLOBAL STATE
// =============================================================================
// These globals are accessed by all worker threads.
// Most are only written by the main thread, read by workers (safe).
// Counters use Interlocked operations (atomic, thread-safe).

static EvolveTask* g_tasks = NULL;      // Task array (set by main thread before signaling)
static int g_task_count = 0;            // Number of tasks this batch

static volatile LONG g_next_task = 0;   // Next task index to grab (atomic counter)
static volatile LONG g_done_count = 0;  // Number of completed tasks (atomic counter)

static HANDLE* g_worker_threads = NULL; // Array of thread handles
static int g_num_workers = 0;           // Number of worker threads
static volatile int g_pool_shutdown = 0;// Flag: 1 = workers should exit

// Synchronization primitives
static HANDLE g_work_semaphore = NULL;  // Counts available tasks (workers wait on this)
static HANDLE g_done_event = NULL;      // Signaled when all tasks complete
static CRITICAL_SECTION g_task_lock;    // (Currently unused, kept for future use)

// Per-run settings
static int g_enable_logs = 0;
static int g_enable_timers = 0;

// =============================================================================
// WORKER THREAD FUNCTION
// =============================================================================
// This is what each worker thread runs. It loops forever until shutdown.
//
// Equivalent C++ pseudocode:
//     void worker() {
//         while (!shutdown) {
//             semaphore.wait();           // Block until work available
//             int task = next_task++;     // Grab a task atomically
//             if (task < task_count) {
//                 execute(tasks[task]);
//                 if (++done_count == task_count)
//                     done_event.signal();
//             }
//         }
//     }
//

static unsigned __stdcall worker_thread_func(void* arg) {
    (void)arg;  // Worker ID - unused but available for debugging
    
    while (!g_pool_shutdown) {
        // -----------------------------------------------------------------
        // WAIT FOR WORK
        // -----------------------------------------------------------------
        // WaitForSingleObject decrements the semaphore counter.
        // If counter is 0, this blocks until someone calls ReleaseSemaphore.
        // Timeout of 100ms allows us to check g_pool_shutdown periodically.
        //
        DWORD wait_result = WaitForSingleObject(g_work_semaphore, 100);
        
        // Check shutdown flag (might have been set while we were waiting)
        if (g_pool_shutdown) break;
        
        // Timeout means no work yet - loop back and wait again
        if (wait_result == WAIT_TIMEOUT) continue;
        
        // -----------------------------------------------------------------
        // GRAB NEXT TASK (ATOMICALLY)
        // -----------------------------------------------------------------
        // InterlockedIncrement atomically does: return (++g_next_task)
        // We subtract 1 to get the task index (0-based).
        //
        // Example with 3 workers, 5 tasks:
        //   Worker A: InterlockedIncrement returns 1 ? task 0
        //   Worker B: InterlockedIncrement returns 2 ? task 1
        //   Worker C: InterlockedIncrement returns 3 ? task 2
        //   Worker A (done with 0): returns 4 ? task 3
        //   Worker B (done with 1): returns 5 ? task 4
        //   Worker C (done with 2): returns 6 ? 6 >= 5, no more tasks
        //
        LONG task_idx = InterlockedIncrement(&g_next_task) - 1;
        
        if (task_idx < g_task_count) {
            // -----------------------------------------------------------------
            // EXECUTE THE TASK
            // -----------------------------------------------------------------
            EvolveTask* task = &g_tasks[task_idx];
            
            EvolveSpecie(
                task->specie, task->pop_size,
                (const double**)task->dist, (const int**)task->ranking,
                task->nodes, task->total_stations,
                task->alpha, task->total_stations,
                task->mutation_rate, task->elitism,
                task->add_pct, task->remove_pct, task->swap_pct,
                task->inv_pct, task->scr_pct, 0.5,
                g_enable_logs, g_enable_timers
            );
            
            // -----------------------------------------------------------------
            // SIGNAL COMPLETION
            // -----------------------------------------------------------------
            // Atomically increment done counter. If we're the last one,
            // signal the main thread that all tasks are complete.
            //
            LONG done = InterlockedIncrement(&g_done_count);
            if (done >= g_task_count) {
                SetEvent(g_done_event);  // Wake up main thread
            }
        }
        // If task_idx >= g_task_count, another worker got the last task.
        // Just loop back and wait for more work.
    }
    
    return 0;
}

// =============================================================================
// ThreadPool_Init - Create worker threads
// =============================================================================
// Equivalent C++ pseudocode:
//     std::vector<std::thread> workers;
//     for (int i = 0; i < num_workers; i++)
//         workers.emplace_back(worker_thread_func);
//
// But we also create synchronization primitives here.
//
int ThreadPool_Init(int num_workers) {
    g_num_workers = num_workers;
    g_pool_shutdown = 0;
    
    // Initialize critical section (like std::mutex, but we don't use it yet)
    InitializeCriticalSection(&g_task_lock);
    
    // -----------------------------------------------------------------
    // CREATE SEMAPHORE
    // -----------------------------------------------------------------
    // Semaphore with initial count 0, max count 10000.
    // Workers will wait on this. When we have N tasks, we release N.
    // Think of it as a "work ticket dispenser".
    //
    g_work_semaphore = CreateSemaphore(
        NULL,   // Default security
        0,      // Initial count (no work yet)
        10000,  // Maximum count (more than we'll ever need)
        NULL    // Unnamed
    );
    
    // -----------------------------------------------------------------
    // CREATE EVENT
    // -----------------------------------------------------------------
    // Manual-reset event: stays signaled until explicitly reset.
    // Used to signal "all tasks complete" to main thread.
    //
    g_done_event = CreateEvent(
        NULL,   // Default security
        TRUE,   // Manual reset (not auto-reset)
        FALSE,  // Initially not signaled
        NULL    // Unnamed
    );
    
    if (!g_work_semaphore || !g_done_event) return 0;
    
    // -----------------------------------------------------------------
    // CREATE WORKER THREADS
    // -----------------------------------------------------------------
    // _beginthreadex is preferred over CreateThread for C code because
    // it properly initializes the C runtime library for each thread.
    //
    g_worker_threads = (HANDLE*)malloc(num_workers * sizeof(HANDLE));
    if (!g_worker_threads) return 0;
    
    for (int i = 0; i < num_workers; i++) {
        unsigned tid;  // Thread ID (we don't use it)
        
        g_worker_threads[i] = (HANDLE)_beginthreadex(
            NULL,                           // Default security
            0,                              // Default stack size
            worker_thread_func,             // Thread function
            (void*)(intptr_t)i,             // Argument (worker ID)
            0,                              // Run immediately (not suspended)
            &tid                            // Receives thread ID
        );
        
        if (!g_worker_threads[i]) {
            // Cleanup on failure
            g_pool_shutdown = 1;
            for (int j = 0; j < i; j++) {
                WaitForSingleObject(g_worker_threads[j], INFINITE);
                CloseHandle(g_worker_threads[j]);
            }
            free(g_worker_threads);
            g_worker_threads = NULL;
            return 0;
        }
    }
    
    // Workers are now running and waiting on the semaphore
    return 1;
}

// =============================================================================
// ThreadPool_Run - Submit tasks and wait for completion
// =============================================================================
// This is called once per generation from the main thread.
//
// Equivalent C++ pseudocode:
//     task_queue = tasks;
//     semaphore.release(count);  // Wake workers
//     done_event.wait();         // Block until all done
//
void ThreadPool_Run(EvolveTask* tasks, int count, int enable_logs, int enable_timers) {
    // Set up this batch
    g_tasks = tasks;
    g_task_count = count;
    g_next_task = 0;        // Reset task counter
    g_done_count = 0;       // Reset completion counter
    g_enable_logs = enable_logs;
    g_enable_timers = enable_timers;
    
    // Reset the "all done" event (it might still be signaled from last run)
    ResetEvent(g_done_event);
    
    // -----------------------------------------------------------------
    // WAKE UP WORKERS
    // -----------------------------------------------------------------
    // ReleaseSemaphore adds 'count' to the semaphore, waking up to
    // 'count' waiting workers. If we have 8 workers and 100 tasks,
    // all 8 workers wake up and start grabbing tasks.
    //
    ReleaseSemaphore(g_work_semaphore, count, NULL);
    
    // -----------------------------------------------------------------
    // WAIT FOR ALL TASKS TO COMPLETE
    // -----------------------------------------------------------------
    // Block here until the last worker calls SetEvent(g_done_event).
    //
    WaitForSingleObject(g_done_event, INFINITE);
}

// =============================================================================
// ThreadPool_Destroy - Clean shutdown
// =============================================================================
// Equivalent C++ pseudocode:
//     shutdown = true;
//     for (auto& t : workers) t.join();
//

void ThreadPool_Destroy(void) {
    if (!g_worker_threads) return;
    
    // Signal workers to exit
    g_pool_shutdown = 1;
    
    // Wake all workers so they can see the shutdown flag and exit.
    // Without this, they'd be stuck waiting on the semaphore forever.
    ReleaseSemaphore(g_work_semaphore, g_num_workers, NULL);
    
    // Wait for each worker to finish (like std::thread::join)
    for (int i = 0; i < g_num_workers; i++) {
        WaitForSingleObject(g_worker_threads[i], INFINITE);
        CloseHandle(g_worker_threads[i]);  // Release the handle (no RAII!)
    }
    
    free(g_worker_threads);
    g_worker_threads = NULL;
    
    // Clean up synchronization primitives
    CloseHandle(g_work_semaphore);
    CloseHandle(g_done_event);
    DeleteCriticalSection(&g_task_lock);
}
