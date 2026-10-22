# Integrated OS Toolkit and Benchmarking Suite — Project Knowledge Base

> **Purpose of this file:** A living technical reference for the project. It records the project's goals, current implementation, design decisions, run instructions, known limitations, and remaining work so that team members can continue development consistently.
>
> **Status snapshot:** Review 1 completed. Stabilization, the page-replacement, memory-allocation, POSIX-demonstration, IPC, deadlock-avoidance, disk-scheduling and file-allocation milestones, and the Phase I generalized benchmarking and final-integration milestone are implemented and verified. All five algorithm-comparison modules report through one shared CSV/chart layer. Wall-clock runtime benchmarking remains out of scope (backlog B5).

## 1. Project Overview

**Project name:** Integrated OS Toolkit and Benchmarking Suite  
**Short description:** A modular, interactive Operating Systems toolkit written in C for Linux/POSIX. It is intended to combine OS algorithm simulations, practical OS programming demonstrations, and automatic performance-result reporting in a single command-line application.

**Primary repository:** <https://github.com/adityaDev-2005/Integrated-OS-Toolkit>  
**Primary language:** C11  
**Build tools:** GCC and Make  
**Graphing:** Python 3 and Matplotlib  
**Target environment:** Linux, including Linux Mint/Ubuntu; other POSIX-like environments may need minor adjustments.

### The core idea

Rather than maintaining a separate program for each OS algorithm, the toolkit provides one menu-driven application. Each algorithm-comparison module follows the same pattern: the user enters one workload, the program evaluates every implemented policy on copies of that same workload, displays a comparison, writes it to CSV, and triggers Python/Matplotlib to generate a graph automatically.

The project does **not** propose new CPU-scheduling algorithms. Its engineering contribution is the integration of existing algorithms, shared inputs, consistent metric calculation, repeatable comparison, export, visualization, and the combination of simulations with real Linux/POSIX demonstrations.

## 2. Problem Statement and Motivation

Operating Systems courses introduce algorithms such as CPU scheduling, page replacement, memory allocation, deadlock avoidance, disk scheduling, and file allocation. If each topic is implemented as an isolated program, users may need to re-enter inputs repeatedly, calculate metrics manually, and prepare graphs separately. That process makes comparisons less convenient and creates opportunities for inconsistent inputs or arithmetic errors.

The proposed toolkit addresses this through:

- A single command-line interface and modular source structure.
- Reusable input and reporting patterns.
- Common workloads for comparing algorithms within a module.
- Automatic calculation of applicable performance metrics.
- CSV output that can be inspected or processed by other tools.
- Automatic graph generation from the saved results.
- POSIX programming demonstrations that connect OS theory to actual system APIs.

Existing educational simulators may already provide some of these capabilities. The project's distinction is the specific combination and implementation in this modular C toolkit, not a claim that all individual features are unique.

## 3. Objectives

1. Build one modular C application for multiple Operating Systems concepts.
2. Implement classic OS algorithms and make their behaviour observable.
3. Compare algorithms using identical inputs within each module.
4. Calculate appropriate per-process and aggregate metrics automatically.
5. Display execution timelines or other useful intermediate results.
6. Export comparison data to CSV and generate plots automatically.
7. Demonstrate Linux/POSIX programming through process APIs, IPC, threads, and synchronization.
8. Keep the code modular so new algorithms and modules can be added without rewriting the complete application.
9. Document test cases, assumptions, setup, and usage for project demonstration and future maintenance.

## 4. Scope and Status at a Glance

The statuses below record the current state of each module. Do not describe a module as complete unless its implementation and tests have been verified.

| Module / capability | Status at current milestone | Notes |
|---|---|---|
| Main CLI and menu | Implemented | Text-based menu dispatches to CPU scheduling, page replacement, memory allocation, the POSIX demonstration, IPC, deadlock avoidance, disk scheduling, file allocation, About and Exit; the final layout is options 1-10. |
| FCFS CPU scheduling | Implemented | Non-preemptive, ordered by arrival time. |
| SJF CPU scheduling | Implemented | Non-preemptive; shortest burst among processes that have arrived. |
| Priority CPU scheduling | Implemented | Non-preemptive; the priority ordering (smaller or larger number first) is selected at runtime. |
| Round Robin CPU scheduling | Implemented | Preemptive time-slicing using a circular ready queue and user-provided quantum. |
| CPU scheduling metrics and timelines | Implemented | Per-process CT, TAT, WT, RT, plus averages and Gantt-style timeline. |
| CPU comparison summary | Implemented | Four algorithms use copies of the same workload. |
| CPU scheduling CSV export | Implemented | Writes `data/results/cpu_scheduling.csv` under the project root. |
| Automatic graph generation | Implemented | C program triggers the Python plotting script; PNG saved locally. A failure is reported without discarding the CSV. |
| CPU scheduling regression tests | Implemented | `make test` runs eight scheduling workloads and compares the generated CSV with fixtures in `tests/cases/`. |
| Page replacement | Implemented | FIFO, LRU and LFU over one shared reference string and frame count; each policy starts with empty frames; hits and faults are reported. |
| Page replacement regression tests | Implemented | Five additional fixtures compare the reported hit and fault counts. |
| Memory allocation | Implemented | First Fit, Best Fit and Worst Fit over one shared block list and request list; exact splitting, fragmentation and allocation failures are reported. |
| Memory allocation regression tests | Implemented | Five fixtures compare the full allocation report, including failures, exact fits, repeated splitting and policy divergence. |
| POSIX process demonstration | Implemented | Real `fork()`, `execl()`/`execlp()`, `wait()`/`waitpid()`, `getpid()`/`getppid()`, file APIs and `opendir()`/`readdir()`. |
| POSIX regression tests | Implemented | One fixture checks required markers, parent/child PID linkage, temporary-file removal and an untouched working directory. |
| Deadlock avoidance | Implemented | Banker's algorithm with Allocation, Maximum, Need and Available, a safety check and a safe sequence when one exists; inconsistent states are rejected. |
| Deadlock avoidance regression tests | Implemented | Five fixtures cover a safe state, a partially progressing unsafe state, a state with several valid sequences, a rejected allocation and an all-zero state. |
| Disk scheduling | Implemented | FCFS, SSTF and SCAN over one shared request queue, initial head position and cylinder bounds; service order and total head movement per policy are compared. |
| Disk scheduling regression tests | Implemented | Five fixtures compare the full policy report and one checks the rejected cylinder values. |
| File allocation | Implemented | Contiguous, linked and indexed allocation on independent copies of one simulated block disk, with allocation, deletion, retrieval, block usage, failures and wasted space reported per method. |
| File allocation regression tests | Implemented | Six fixtures compare the full report block and one checks the rejected input values; the contiguous fragmentation and indexed capacity limits are pinned explicitly. |
| IPC and producer-consumer | Implemented | `pipe()` with `fork()` for inter-process communication; pthreads with POSIX semaphores and a bounded ring buffer for producer-consumer; shared memory excluded by decision Q1. |
| IPC regression tests | Implemented | Four fixtures check the stable producer-consumer and pipe markers, the rejected-input paths, and an edge configuration with more threads than items. |
| General-purpose benchmarking layer | Implemented | Shared CSV writer and plot driver in `src/benchmark/`, used by the five algorithm-comparison modules. Banker's algorithm, POSIX and IPC stay console-only. |
| Module CSV and chart reporting (paging, memory, disk, file) | Implemented | `data/results/<module>.csv` and `<module>_comparison.png`; CSV and plot failures are reported without discarding algorithm results. |
| Reporting and integration regression tests | Implemented | Five CSV comparisons, one menu-dispatch chain, two CSV-failure and two plot-failure checks; `make test` runs 52 checks. |
| Real wall-clock algorithm runtime benchmarking | Not currently implemented | Current comparisons are simulated metrics, not algorithm execution-time measurements; runtime benchmarking remains backlog B5. |

### Scope discipline and simulation boundaries

Only the modules, algorithms and demonstrations named in this knowledge base are
implemented. No unrelated OS module or feature is added.

- CPU scheduling, page replacement, memory allocation, deadlock avoidance, disk
  scheduling and file allocation are simulations of OS behavior. Their metrics
  come from the entered workload. The file allocation module works on an
  in-memory block disk and never reads or modifies a real filesystem.
- The POSIX demonstration executes actual Linux system calls. Its process IDs,
  wait statuses, file descriptors and directory entries are real runtime values.
- Simulated metrics are never presented as measured runtime. The POSIX
  demonstration is kept in a separate menu path and module (`src/posix/`).

### Scope decision Q1 — shared memory

Shared memory is excluded from the project scope (decision recorded
2026-10-22). Phase E implements pipes, pthreads and POSIX semaphores only. No
shared-memory segment, System V or POSIX shared-memory API, or `mmap()`-backed
shared buffer is used. The status table and the Phase E description follow this
decision, so the earlier "shared memory if included" condition no longer applies.


## 5. Current CPU Scheduling Workflow

1. The user launches the application and selects **Run CPU Scheduling Comparison**.
2. The application asks for the number of processes (currently 1–100).
3. It asks which priority ordering to use (smaller or larger numbers first), then for every process it asks for arrival time, burst time, and priority.
4. It asks for the Round Robin time quantum.
5. Input is validated; invalid values are re-prompted.
6. Processes are sorted by arrival time while preserving input order for arrival-time ties.
7. The workload is copied so each algorithm begins with the same process data and fresh result fields.
8. The program runs FCFS, non-preemptive SJF, non-preemptive Priority, and Round Robin.
9. For each algorithm, it prints a Gantt-style timeline and a process table.
10. It calculates and prints average waiting time, average turnaround time, and average response time.
11. It prints a four-algorithm comparison table.
12. It saves the aggregate comparison to `data/results/cpu_scheduling.csv` under the project root, independent of the current working directory.
13. Unless `OS_TOOLKIT_NO_PLOT` is set, it invokes `scripts/plot_results.py`, which reads the CSV and writes the PNG graph.
14. If a desktop session is available and `OS_TOOLKIT_NO_OPEN` is not set, the C program opens the image with `xdg-open`; otherwise the viewer is skipped.
15. Control returns to the main menu.

### Current input constraints

| Input | Current validation range |
|---|---:|
| Number of processes | 1–100 |
| Arrival time | 0–1,000,000 |
| Burst time | 1–1,000 |
| Priority | 1–1,000,000 |
| Priority ordering | 1 (smaller first) or 2 (larger first) |
| Round Robin quantum | 1–1,000 |

Priority convention: **selected at runtime**; option 1 treats a smaller number as higher priority, option 2 treats a larger number as higher priority.

### Current page replacement workflow

1. The user selects **Page Replacement Comparison** (menu option 2).
2. The application asks for the number of frames (1–100) and the number of page references (1–1000).
3. It reads one page number (0–999) per reference; invalid values are re-prompted.
4. FIFO, LRU and LFU each run over the same reference string with their own frame array, and every policy starts with empty frames.
5. Each policy reports page hits and page faults, and the three results are printed in one comparison table.
6. LFU evicts the page with the smallest access count; ties go to the page that was loaded earliest.
7. Control returns to the main menu.

The module writes `data/results/page_replacement.csv` and generates
`data/results/page_replacement_comparison.png` with hits and faults per policy.

### Current memory allocation workflow

1. The user selects **Memory Allocation Comparison** (menu option 3).
2. The application asks for the number of memory blocks (1–100) and the size of each block (1–1,000,000).
3. It asks for the number of requests (1–100) and the size of each request (1–1,000,000); invalid values are re-prompted.
4. First Fit, Best Fit and Worst Fit each start from a fresh copy of the same block list and process the same requests in order.
5. First Fit chooses the first block with enough free space. Best Fit chooses the smallest sufficient block. Worst Fit chooses the largest free block. Ties go to the lowest-numbered block.
6. An allocation takes exactly the requested size from the chosen block; the remaining free space stays with the block for later requests.
7. A request that no block can satisfy is reported as failed and consumes no space.
8. The report shows the chosen block for every request, the remaining free space of every block, and a summary with allocated count, failed count, internal fragmentation and the largest free block (external fragmentation).
9. Control returns to the main menu.

Under exact splitting, allocated size equals requested size, so internal fragmentation is zero. No alignment overhead or minimum allocation unit is modeled. External fragmentation is reported as the largest free block.

The module writes `data/results/memory_allocation.csv` and generates
`data/results/memory_allocation_comparison.png` with allocation, failure and
fragmentation metrics per policy.

### Current POSIX demonstration workflow

1. The user selects **POSIX Process and File Demo** (menu option 4). No further input is needed.
2. The program prints the parent's real `getpid()` and `getppid()`.
3. It creates a child with `fork()` and reaps it with `waitpid()`; the child exits with status 42. A second child is reaped with `wait()`; it exits with status 7.
4. A child calls `execlp("echo", ...)`, replacing its image; the parent reaps it with status 0.
5. Another child calls `execl()` on a missing path. The failure is reported from `errno` and the child exits with status 127.
6. `mkstemp()` creates one temporary file under `/tmp`. The program demonstrates `open()`, `write()`, `fcntl(F_GETFL/F_SETFL)`, `lseek()`, `read()`, `stat()` and `close()` on that file. Content is `hello world` and the size is 11 bytes.
7. The file is unlinked and all descriptors are closed before the function returns.
8. `opendir("/tmp")` and up to five `readdir()` calls demonstrate directory iteration, followed by `closedir()`.
9. Control returns to the main menu.

All file writes are confined to the temporary file. The module does not create or modify project files. Pipes and other IPC are deliberately absent from this module; they belong to Phase E. Shared memory is excluded from the project scope by decision Q1, so Phase E uses pipes and pthreads with POSIX semaphores only.

### Current disk scheduling workflow

1. The user selects **Disk Scheduling Comparison** (menu option 7).
2. The application asks for the minimum cylinder (0-9998), the maximum cylinder (minimum+1 to 9999), the initial head position and the request count (1-200).
3. It reads one cylinder number per request; the head and every request are validated against the configured bounds, and invalid values are re-prompted.
4. FCFS, SSTF and SCAN each process the same queue from the same initial head position, keeping independent service flags, paths and movement totals.
5. FCFS serves the queue in order. SSTF serves the nearest pending request, breaking distance ties by the lower cylinder and cylinder ties by the earlier queue entry. SCAN starts toward the maximum cylinder, always reaches that bound before reversing (the boundary travel is counted) and then serves the remaining requests in descending order, stopping at the last one. This is conventional SCAN, not LOOK.
6. Each policy reports its service order as a head path that starts at the initial head, plus the total head movement in cylinders. The three totals are then repeated in one comparison table.
7. Control returns to the main menu.

The module writes `data/results/disk_scheduling.csv` and generates
`data/results/disk_scheduling_comparison.png` with total head movement per
policy.

### Current file allocation workflow

1. The user selects **File Allocation Simulation** (menu option 8).
2. The application asks for the simulated disk block count (1–128), the block size in bytes (8–4096) and the number of file operations (1–64).
3. For every operation it asks for the type (1 allocate, 2 delete, 3 retrieve) and the file ID (1–32); an allocation also asks for the file size in bytes (1–1000000). Invalid values are re-prompted.
4. Contiguous, linked and indexed allocation each run on an independent copy of the same empty disk and receive the same operation sequence.
5. Contiguous allocation needs `ceil(size / block_size)` data blocks and takes the first free contiguous run that fits.
6. Linked allocation reserves a 4-byte next pointer in every data block, so each block carries `block_size - 4` bytes of file data; it needs `ceil(size / (block_size - 4))` blocks, taken in ascending free-block order.
7. Indexed allocation needs `ceil(size / block_size)` data blocks plus one index block. The index block is allocated first and holds `block_size / 4` 4-byte pointers; a file needing more data blocks is rejected.
8. Allocation is atomic: every check runs before the disk changes, so a failed request leaves the volume untouched. The report names the reason (duplicate ID, insufficient free blocks, no contiguous run, index capacity, missing file).
9. Deletion frees all data and index blocks. Retrieval prints the file size and the stored block mapping, including the index block for indexed files.
10. Each method reports its operations, block usage, allocation/deletion/retrieval counts and wasted space, followed by shared notes.
11. Control returns to the main menu.

The disk is a simulated array of blocks in memory. The module never reads or
modifies a real filesystem, and its banner says so. Wasted space is
`allocated block bytes - requested bytes` over the active files, so it includes
last-block slack, link-pointer slots and index blocks; index blocks are also
reported as the metadata-block count. The module writes
`data/results/file_allocation.csv` and generates
`data/results/file_allocation_comparison.png` covering block usage, operation
counters, bytes and waste.

### Algorithm definitions and assumptions

#### FCFS — First Come First Served

- Non-preemptive.
- Processes run in arrival order.
- A process runs to completion once it receives the CPU.
- If the CPU is idle before the next process arrives, the simulated time advances to that arrival.

#### SJF — Shortest Job First

- Non-preemptive variant.
- At each dispatch, choose the shortest burst among processes that have already arrived and have not completed.
- If no process is ready, advance simulated time to the next arrival.
- Ties are resolved deterministically using earlier arrival time and then process ID/input order.

#### Priority Scheduling

- Non-preemptive variant.
- At each dispatch, choose the ready process with the smallest numeric priority value.
- If priorities tie, earlier arrival time and then process ID/input order determine the selection.
- Priority starvation is a known property of this scheduling policy; aging is not currently implemented.

#### Round Robin (RR)

- Preemptive time-slicing algorithm.
- A process executes for at most one time quantum per turn, or until it completes.
- Processes arriving during a time slice join the ready queue before the unfinished process is re-queued.
- A circular queue is used for ready processes.
- The quantum affects responsiveness and the number of scheduling slices; it does not model measured operating-system context-switch overhead.

### Metric formulas

For each process:

- **Completion Time (CT):** simulated time at which the process finishes.
- **Turnaround Time (TAT):** `CT - Arrival Time`.
- **Waiting Time (WT):** `TAT - Burst Time`.
- **Response Time (RT):** `First CPU Start Time - Arrival Time`.

The averages are arithmetic means across all processes in the workload:

- `Average WT = sum of all process WT / number of processes`
- `Average TAT = sum of all process TAT / number of processes`
- `Average RT = sum of all process RT / number of processes`

These are simulated scheduling metrics. They are **not** wall-clock runtimes of the algorithm implementations.

## 6. Data Storage and Visualization

No database is used. Flat CSV files are sufficient for the current workloads and can be read by Python, a spreadsheet program, or other analysis tools.

### CSV output

Every algorithm-comparison module writes one CSV to
`data/results/<module>.csv` through the shared writer in
`src/benchmark/csv_logger.c`:

- The first column is always `algorithm`.
- Metric columns use snake_case names.
- Workload-context columns follow the metrics (for example the Round Robin
  quantum, frame count or disk geometry).
- Floating-point values are written with two decimals.
- Each file contains one aggregate result row per algorithm.
- There is no timestamp and no runtime column: every value is a simulated result
  of the entered workload, never measured implementation execution time.

| Module | File | Columns after `algorithm` |
|---|---|---|
| CPU scheduling | `cpu_scheduling.csv` | `avg_waiting_time`, `avg_turnaround_time`, `avg_response_time`, `quantum`, `process_count` |
| Page replacement | `page_replacement.csv` | `hits`, `faults`, `frame_count`, `reference_count` |
| Memory allocation | `memory_allocation.csv` | `allocated_requests`, `failed_requests`, `internal_fragmentation`, `largest_free_block`, `block_count`, `request_count` |
| Disk scheduling | `disk_scheduling.csv` | `total_head_movement`, `request_count`, `initial_head`, `min_cylinder`, `max_cylinder` |
| File allocation | `file_allocation.csv` | `used_blocks`, `free_blocks`, `active_files`, `allocations_ok`, `allocations_failed`, `deletions_ok`, `deletions_failed`, `retrievals_ok`, `retrievals_failed`, `requested_bytes`, `allocated_bytes`, `wasted_bytes`, `metadata_blocks`, `block_count`, `block_size`, `operation_count` |

The CPU CSV keeps the exact schema and values it had before Phase I. Banker's
algorithm, the POSIX demonstration and the IPC demonstration are not algorithm
comparisons, so they remain console-only and produce no CSV.

### Graph output

**Script:** `scripts/plot_results.py`  
**Output:** `data/results/<module>_comparison.png`

The script takes the module name as its argument (the default is
`cpu_scheduling`), validates that the CSV exists, contains the required columns
and holds valid numeric rows, then writes one PNG:

| Module | Chart |
|---|---|
| CPU scheduling | Three side-by-side bar charts: Average Waiting Time, Average Turnaround Time and Average Response Time (unchanged layout). |
| Page replacement | Page hits and page faults per policy. |
| Memory allocation | 2x2 panels: allocated vs failed requests, internal fragmentation, largest free block, and block/request counts. |
| Disk scheduling | Total head movement per policy. |
| File allocation | 2x2 panels: block usage, operation counters, requested vs allocated bytes, and wasted bytes with metadata blocks. |

Every plot compares the policies of one module for the same user-supplied workload. Lower values are preferable for each individual metric, but the policy best suited to a use case depends on the metric and workload.

### Automatic C-to-Python integration

After successfully writing the CSV, the C program invokes the plotting script using absolute paths derived from the project root. `src/common/common.c` resolves that root from `OS_TOOLKIT_ROOT` when set, otherwise from the directory two levels above `/proc/self/exe`; the application can therefore be started from any working directory.

The launch sequence is:

1. Run `scripts/plot_results.py` with `.venv/bin/python` when that interpreter exists and is executable.
2. Fall back to `python3`.
3. Report clearly when the graph could not be generated; the CSV remains valid.

The PNG is opened with `xdg-open` only when `OS_TOOLKIT_NO_OPEN` is unset and either `DISPLAY` or `WAYLAND_DISPLAY` is present. `OS_TOOLKIT_NO_PLOT=1` skips plotting entirely.

If graph generation fails, check that:

- Matplotlib is installed in `.venv` or in the system Python environment used by the program.
- The CSV was created with the expected columns and rows.
- The plotting script exists and can be run directly for debugging.

A failed CSV open, write or close prints
`CSV export failed; the algorithm results above are still valid.` and skips
plotting. A failed plot prints `Could not generate the graph.` and states that
the CSV and the algorithm results are still valid. In both cases the console
comparison is already complete and the module exits normally.

## 7. Architecture

The project follows four layers:

1. **User interface layer:** `src/main.c`, the menu, and validated input handling.
2. **Algorithm/module layer:** independent modules for CPU scheduling, page replacement, memory allocation, disk scheduling, file allocation, deadlock avoidance, the POSIX demonstration and the IPC demonstration.
3. **Metrics and data layer:** metric calculation and CSV output, provided by the shared benchmarking module in `src/benchmark/`.
4. **Visualization layer:** Python/Matplotlib reads generated CSV output and produces figures.

The current working execution path is the main CLI; the CPU scheduling, page replacement, memory allocation, disk scheduling and file allocation comparisons with their CSV output and plotting; the POSIX process/file demonstration; the IPC demonstration; and deadlock avoidance. The shared benchmark module is compiled into the binary and used by the five comparison modules; its writer, plot driver and failure handling are exercised by the regression suite.

## 8. Repository Structure

The project contains the following files:

```text
Integrated-OS-Toolkit/
├── AGENT_CONTEXT.md               # continuity notes for resuming a session
├── PLAN.md                        # milestone plan and progress log
├── PROJECT_KNOWLEDGE_BASE.md      # this document
├── Makefile
├── README.md
├── include/
│   ├── banker.h                   # deadlock avoidance entry point
│   ├── benchmark.h                # shared CSV/plot API
│   ├── common.h                   # project-root path helper
│   ├── disk.h                     # disk scheduling entry point
│   ├── file_alloc.h               # file allocation entry point and limits
│   ├── memory.h
│   ├── paging.h
│   ├── process_demo.h             # POSIX demonstration entry point
│   ├── producer_consumer.h        # IPC demonstration entry point
│   └── scheduling.h
├── src/
│   ├── main.c
│   ├── common/
│   │   └── common.c               # project_path()
│   ├── scheduling/
│   │   └── scheduling.c
│   ├── deadlock/
│   │   └── banker.c               # Banker's safety algorithm and rejection rules
│   ├── disk/
│   │   └── disk.c                 # FCFS, SSTF and SCAN comparison
│   ├── file_alloc/
│   │   └── file_alloc.c           # contiguous, linked and indexed allocation
│   ├── paging/
│   │   └── paging.c               # FIFO, LRU and LFU comparison
│   ├── memory/
│   │   └── memory.c               # First, Best and Worst Fit comparison
│   ├── posix/
│   │   ├── process_demo.c         # real fork/exec/wait/file/directory demo
│   │   └── producer_consumer.c    # pthreads + semaphore producer-consumer and pipe demo
│   └── benchmark/
│       ├── benchmark.c            # plot launch and reporting messages
│       └── csv_logger.c           # shared CSV writer
├── scripts/
│   └── plot_results.py
├── tests/
│   ├── test_cases.md              # regression matrix and expected values
│   ├── run_tests.sh               # regression runner used by `make test`
│   └── cases/                     # *.in inputs and *.expected.csv / *.expected.txt fixtures
├── data/
│   └── results/                   # generated locally; git-ignored
└── build/
    └── os_toolkit                # compiled locally; do not commit
```

### Build configuration

At the current milestone, the Makefile builds the main program, the shared benchmark reporting layer, the shared path helper, the scheduling implementation, and the page replacement, memory allocation, deadlock avoidance, disk scheduling, file allocation, POSIX and IPC modules: 12 source files in total. Headers are declared as dependencies, so a header change triggers a rebuild. The compiler flags include:

- `-Wall`
- `-Wextra`
- `-Wpedantic`
- `-std=c11`
- `-pthread`
- `-Iinclude`

The build must be warning-free. `make test` builds the binary and then runs `tests/run_tests.sh`. The source and header lists are complete for the current modules; update them if a module is added (none are planned within scope).

## 9. Setup and Run Instructions

The following instructions target Linux Mint/Ubuntu.

### 1. Clone the repository

```bash
git clone https://github.com/adityaDev-2005/Integrated-OS-Toolkit.git
cd Integrated-OS-Toolkit
```

### 2. Install build and Python prerequisites

```bash
sudo apt update
sudo apt install build-essential python3 python3-venv
```

### 3. Create a Python virtual environment and install Matplotlib

```bash
python3 -m venv .venv
.venv/bin/python -m pip install matplotlib
```

### 4. Compile the C application

```bash
make clean && make
```

Expected executable:

```text
build/os_toolkit
```

### 5. Run the application

```bash
./build/os_toolkit
```

The binary can be started from any directory; generated files always go to the project's `data/results/` directory.

Select option `1` for CPU Scheduling Comparison, then enter the process count, arrival time, burst time and priority for each process, followed by the Round Robin quantum.

### 6. Check locally generated outputs

After a comparison finishes, the application creates the module's files under
`data/results/`:

```text
cpu_scheduling.csv / cpu_scheduling_comparison.png
page_replacement.csv / page_replacement_comparison.png
memory_allocation.csv / memory_allocation_comparison.png
disk_scheduling.csv / disk_scheduling_comparison.png
file_allocation.csv / file_allocation_comparison.png
```

To list them:

```bash
ls -lh data/results/
```

To inspect a CSV:

```bash
cat data/results/cpu_scheduling.csv
```

The application opens the graph automatically only in a graphical session and only when `OS_TOOLKIT_NO_OPEN` is unset. For manual troubleshooting only, run:

```bash
.venv/bin/python scripts/plot_results.py cpu_scheduling
.venv/bin/python scripts/plot_results.py page_replacement
xdg-open data/results/cpu_scheduling_comparison.png
```

## 10. Example Workload for Testing

A documented sample workload is:

| Process | Arrival Time | Burst Time | Priority |
|---|---:|---:|---:|
| P1 | 0 | 5 | 2 |
| P2 | 1 | 3 | 1 |
| P3 | 2 | 8 | 4 |
| P4 | 3 | 6 | 3 |

Round Robin quantum: `3`.

The Review 1 report records the following average metrics for this workload:

| Algorithm | Average WT | Average TAT | Average RT |
|---|---:|---:|---:|
| FCFS | 5.75 | 11.25 | 5.75 |
| SJF (non-preemptive) | 5.25 | 10.75 | 5.25 |
| Priority (non-preemptive) | 5.25 | 10.75 | 5.25 |
| Round Robin | 8.50 | 14.00 | 3.00 |

These values are reproduced from the current Review 1 report. When validating a code change, rerun the workload and independently check the program output against the expected calculations; update this table if the implementation or test assumptions change.

### Minimum regression checks

- `make test` should pass every fixture and integration check (52 checks).
- The program should build with `make clean && make` without compiler warnings.
- Invalid or out-of-range numeric input should be rejected and requested again.
- All four algorithms should receive the same process inputs.
- Idle time before the earliest arrival and gaps between arrivals should be handled.
- SJF and Priority should choose only among processes that have arrived.
- Response time should be measured at the first CPU allocation, not the completion time.
- Round Robin should handle processes that arrive during another process's quantum.
- The comparison should produce four CSV data rows.
- The plot should have all four algorithm names for each of the three metrics.
- Generated CSV and PNG files should be regenerated from the current run, not manually edited.
- Output should land under the project root even when the program is started from another directory.
- The CSV should still be written when plotting is unavailable.
- Every comparison module should write its CSV under `data/results/` even when the binary starts from another directory.
- A blocked CSV destination and a missing plotting script should be reported without discarding the algorithm results.

## 11. Current Limitations and Design Decisions

1. **Simulated metrics only:** Every graph and CSV value comes from the simulated workload (waiting and turnaround times, page hits and faults, fragmentation, head movement, block usage and waste). The program does not measure the C algorithm's real execution time or real OS context-switch overhead.
2. **Non-preemptive SJF and Priority:** Preemptive SJF/SRTF and preemptive Priority are not part of the current implementation.
3. **Priority direction:** The user chooses whether a smaller or larger integer denotes higher priority; the active choice must be stated in the interface.
4. **Tie-breaking:** Deterministic tie rules are used to keep results repeatable; document any future changes to those rules.
5. **Paths:** Output and script paths are resolved from the project root, so the binary can be started from any directory. `OS_TOOLKIT_ROOT` overrides the root when the binary is copied elsewhere.
6. **Desktop dependency:** `xdg-open` is specific to Linux desktop environments. The PNG is still generated on a headless machine; the viewer is skipped when neither `DISPLAY` nor `WAYLAND_DISPLAY` is set.
7. **Linux-specific root discovery:** `project_path()` reads `/proc/self/exe`, so the project targets Linux. On another platform, set `OS_TOOLKIT_ROOT`.
8. **External Python dependency:** The plotting step requires Python and Matplotlib. The C scheduling results can be computed independently, but the graph step depends on the plotting environment.
9. **Generated artifacts:** The CSV, PNG, executable, and virtual environment are runtime/build outputs and should normally remain untracked unless the team deliberately wants to publish sample artifacts.
10. **No database:** Data is currently stored as a simple CSV, which is appropriate for the current scope.
11. **Reporting scope:** CSV and charts cover the five algorithm-comparison modules. Banker's algorithm, the POSIX demonstration and the IPC demonstration remain console-only because they have no alternative algorithms to compare; no artificial metrics are produced for them.
12. **No runtime benchmarking:** There is no `CLOCK_MONOTONIC` or wall-clock timing code. Page faults, waiting time, head movement, allocation failures and similar values are simulation outputs, not measured execution time. Runtime benchmarking remains backlog B5.

## 12. Development Roadmap

The planned work is best done incrementally. Finish and test each module before integrating the next one.

### Phase A — Stabilize the initial milestone

- [x] Main CLI and Makefile foundation.
- [x] Shared CPU scheduling workload input.
- [x] FCFS, non-preemptive SJF, non-preemptive Priority, and Round Robin.
- [x] Per-process CT/TAT/WT/RT calculations and averages.
- [x] Gantt-style timelines and comparison table.
- [x] CSV export for the four scheduling algorithms.
- [x] Automatic graph generation through Python/Matplotlib.
- [x] Add or verify explicit unit/regression tests for edge cases (`make test`, eight fixtures plus a root-override check).
- [x] Ensure graph generation reports failures clearly and works from a fresh clone.
- [x] Confirm that README setup instructions match the final code.

### Phase B — Page replacement

Implement FIFO, LRU and LFU using one shared page-reference string and frame count. Report page hits and faults and compare all policies on identical input. Define the LFU tie-breaking rule and decide whether frames begin empty for every algorithm (they normally should for a fair independent comparison). Consider visualizing fault counts after the CSV/plot workflow is generalized.

Status: implemented and verified (M1). Every policy starts with empty frames. LFU evicts the smallest access count, then the page loaded earliest. CSV output and charts were added in Phase I through the shared reporting layer.

### Phase C — Memory allocation

Implement First Fit, Best Fit and Worst Fit with common memory-block sizes and process/request sizes. Display block allocation, internal/external fragmentation metrics if defined, and failures to allocate. Clearly define whether blocks are split and how remaining space is represented.

Status: implemented and verified (M2). All three policies start from a fresh copy of the same block list and process the same requests. Allocation is an exact split: the allocated size equals the requested size and the remaining free space stays with the block, so internal fragmentation is zero. External fragmentation is reported as the largest free block. Failures are reported per request and counted. CSV output and charts were added in Phase I.

### Phase D — POSIX process management

Build demonstrations using actual Linux/POSIX APIs, such as:

- `fork()` to create a child process.
- `exec()` family functions to replace a process image.
- `wait()` / `waitpid()` to collect child status.
- `getpid()` / `getppid()` for process identification.
- File APIs such as `open()`, `read()`, `write()`, `close()`, `lseek()`, `stat()`, and `fcntl()`.
- Directory operations using `opendir()` and `readdir()`.

Keep system-call demonstrations clearly separate from simulations of OS algorithms.

Status: implemented and verified (M3). The module uses real `fork()`, `execl()`/`execlp()`, `wait()`/`waitpid()` and process-ID calls. One temporary file created with `mkstemp()` demonstrates `open`, `write`, `close`, `lseek`, `read`, `stat` and `fcntl`; it is unlinked on every path. `opendir()`/`readdir()` list `/tmp` and the stream is closed. The regression suite checks PID linkage, the failed-exec path and cleanup. No pipes or shared memory are included.

### Phase E — IPC and synchronization

Scope decision Q1 excludes shared memory. Phase E uses pipes for inter-process
communication and pthreads with POSIX semaphores for the producer-consumer
demonstration.

Implement and explain:

- Pipes for inter-process communication, including correct pipe-end closure,
  EOF handling and child-process cleanup.
- Pthreads for multithreading.
- POSIX semaphores and a bounded-buffer producer-consumer example with
  configurable buffer size, item count and thread counts.
- Correct synchronization, cleanup, and error handling.

Document the difference between a demonstration and a production-ready concurrent system.

Status: implemented and verified (M4) in `src/posix/producer_consumer.c`, reachable from menu option 5. The producer-consumer uses a bounded ring buffer, an `empty` and a `full` semaphore for capacity, a mutex for the ring and a second mutex for the bookkeeping; producers take unique item IDs, consumers reserve work so no thread needs a fixed quota, and the module reports produced/consumed counts, duplicates, missing and invalid IDs, and the empty ring after every thread is joined. The pipe demonstration closes the unused end in each process, treats `read()` returning 0 as EOF, retries `EINTR`, reaps the child with `waitpid()` and checks its status. Both are demonstrations, not production concurrency frameworks: there is no work stealing, no back-pressure policy, no cancellation and no timeout handling.

### Phase F — Deadlock avoidance

Implement Banker's algorithm with allocation, maximum, need and available structures. Report whether the system is safe and print a safe sequence when one exists. Validate dimensions and reject impossible or invalid input.

Status: implemented and verified (M5) in `src/deadlock/banker.c`, reachable from menu option 6. The module reads a process count and resource-type count (1-20 each), the Allocation and Maximum matrices and the Available vector; all numeric input is validated and re-prompted. `Need` is derived as `Maximum - Allocation`. An Allocation entry above the same process's Maximum rejects the state before any analysis, because the claim itself would be inconsistent. The safety algorithm starts `Work` at `Available` and repeatedly finishes the lowest-numbered process whose Need fits, adding its Allocation back to `Work`; it reports SAFE with a safe sequence or UNSAFE with the unfinished processes. An unsafe verdict is never reported as a deadlock: the module states that no safe completion order is guaranteed. Request vectors, resource release and prevention or detection algorithms are outside this phase. The state is a teaching simulation, not a real kernel resource table.

### Phase G — Disk scheduling

Implement FCFS, SSTF and SCAN. Use one common request queue and initial head position, document disk-cylinder bounds and SCAN direction, calculate total head movement, and compare the results.

Status: implemented and verified (M6) in `src/disk/disk.c`, reachable from menu option 7. One request queue, one initial head and configurable minimum/maximum cylinder bounds are read once; the head and every request are validated against those bounds. FCFS serves queue order. SSTF serves the nearest pending request and breaks ties deterministically: equal distance goes to the lower cylinder number, and equal cylinder numbers go to the earlier queue entry. SCAN starts toward the maximum cylinder, serves every pending request on the way up, always reaches the bound before reversing (the boundary travel is included in the total) and then serves the remaining requests downwards, stopping at the last one because no work remains beyond it; this is conventional SCAN, not LOOK. Each policy reports its head path and total head movement, and one comparison table repeats the three totals. Six regression fixtures cover mixed requests, one-sided requests, boundary requests, duplicates, requests at the head, SSTF distance ties and rejected cylinder values. CSV output and charts were added in Phase I.

### Phase H — File allocation

Demonstrate contiguous, linked and indexed allocation using an in-memory or clearly specified simulated disk-block model. Explain the assumptions and show allocation/retrieval behaviour. Do not imply that the program modifies a real filesystem unless it actually does.

Status: implemented and verified (M7) in `src/file_alloc/file_alloc.c`, reachable from menu option 8. The disk has 1-128 blocks of 8-4096 bytes; files are sized in bytes and identified by an ID of 1-32, and a run holds 1-64 operations. Each method runs on an independent copy of the same empty disk with the same operations. Contiguous allocation uses `ceil(size / block_size)` data blocks from the first free run that fits. Linked allocation reserves a 4-byte next pointer in every data block, so `block_size - 4` bytes per block carry data, and takes `ceil(size / (block_size - 4))` blocks in ascending order. Indexed allocation uses `ceil(size / block_size)` data blocks plus one index block allocated first; the index holds `block_size / 4` 4-byte pointers and an over-capacity file is rejected. Allocation is atomic, deletion frees data and index blocks, and retrieval prints the stored mapping. Wasted space is allocated block bytes minus requested bytes over the active files, so it includes last-block slack, link-pointer slots and index blocks. The module never reads or modifies a real filesystem and says so in its interface and documentation. Seven regression fixtures cover mixed operations, exhausted space, deletion and reuse, contiguous fragmentation, indexed capacity, the minimum disk and invalid input; CSV output and charts were added in Phase I.

### Phase I — Generalized benchmarking and final integration

- Extract common logging into the planned `src/benchmark/` module.
- Define a consistent CSV schema for each type of module while retaining module-specific metrics.
- Add reusable charting patterns, labels, and error handling.
- Add tests and documented sample inputs for every implemented module.
- Update menu, Makefile, headers, README and this knowledge base as features become functional.
- Conduct an integration test from a clean clone.
- Prepare a final demonstration and report that distinguish implemented features from stretch goals.

Status: implemented and verified (M8). The shared layer lives in
`include/benchmark.h` and `src/benchmark/`; CPU scheduling, page replacement,
memory allocation, disk scheduling and file allocation write `<module>.csv` and
`<module>_comparison.png`, while Banker's algorithm, POSIX and IPC stay
console-only. No wall-clock timer was added. `make clean && make` is
warning-free, `make test` passes 52 checks, and a clean-tree copy builds and
passes the same suite. The menu is final at options 1-10.

## 13. Potential Future Improvements

These are optional enhancements, not current capabilities:

- Support command-line flags or batch input for reproducible experiments.
- Use a configuration file or input file for large test cases.
- Let users compare results from multiple workloads.
- Separate simulated scheduling metrics from actual runtime benchmarks in the UI and CSV schemas.
- Add more scheduling policies only when they fit the project scope and can be tested properly.
- Store versioned example datasets or sample graphs only if useful for the repository; generated output need not be committed by default.

## 14. Team Responsibilities

The Review 1 responsibility split is recorded as follows. These are areas of ownership, not a restriction on shared design reviews and integration work.

| Team member | Initial responsibility |
|---|---|
| Aditya Asutosh Mishra | Repository/project structure, CLI main menu, CPU scheduling engine and metrics. |
| Sai Swarup Shroff | Benchmarking and visualization, CSV export, plotting script, testing and documentation. |
| Siddhant Chauhan | Page replacement and memory allocation modules, POSIX process and IPC demonstrations. |

All team members should participate in design discussions, code review and integration testing.

## 15. Git and Repository Practices

> **Current instruction:** do not commit or push during milestone work. Commit
> only after the complete project is implemented and the user explicitly
> authorizes it. The practices below apply when that authorization is given.

- Work from the project root, `Integrated-OS-Toolkit/`.
- Use `git status --short` before staging or committing.
- Use explicit `git add` paths when only certain source files should be committed.
- Review staged changes with `git diff --cached --stat` and `git diff --cached --check`.
- Do not accidentally commit `.venv/`, the executable under `build/`, or generated CSV/PNG artifacts if the team intends to keep them local.
- Keep backup files such as `src/scheduling/scheduling_old.c` out of commits unless the team deliberately decides to preserve them as part of the repository.
- Commit logical units of work with descriptive messages.
- Pull/rebase carefully and resolve conflicts rather than using force-push as a shortcut.
- Update the README and this knowledge base when user commands, dependencies, output paths or module status change.

Example workflow:

```bash
git status --short
git diff --check
git add Makefile include/scheduling.h src/main.c src/scheduling/scheduling.c scripts/plot_results.py
git diff --cached --stat
git diff --cached --check
git commit -m "Describe the change"
git pull --rebase origin main
git push origin main
```

Only stage the files relevant to the commit. Add a file path to `git add` only when you intentionally want to track that file.

## 16. Demonstration Talking Points

For a review or viva, be prepared to explain:

1. Why common inputs are important for a fair comparison.
2. The differences between FCFS, non-preemptive SJF, non-preemptive Priority and Round Robin.
3. Why turnaround time, waiting time and response time are different.
4. Why the same workload is copied/reset for each algorithm.
5. How Round Robin uses its quantum and ready queue.
6. Why priority direction and tie-breaking rules need to be explicit.
7. How the C program writes module-specific aggregate metrics through the shared CSV writer and invokes Python/Matplotlib.
8. Why the graph measures simulated scheduling metrics rather than real implementation runtime.
9. How the architecture was extended for page replacement, memory allocation and the POSIX demonstration, and how it can be extended further.
10. Why exact block splitting makes internal fragmentation zero and how external fragmentation is reported.
11. Which parts of the toolkit are simulations and which execute real Linux system calls; why the POSIX PID, wait-status and file results are not mocked.
12. How `fork()` return values differ in the parent and child, and why `_exit()` is used after a failed `exec()`.
13. How the POSIX module confines writes to one `mkstemp()` file and cleans up descriptors, children and the temporary file.
14. Which modules report to CSV and charts and which stay console-only, and why.
15. Why SSTF needs a documented tie-break and why conventional SCAN pays for the boundary run that LOOK skips.
16. Why linked allocation needs more blocks than contiguous allocation for the same file, how the index block limits indexed allocation, and why a fragmented disk can fail a contiguous request even when total free space is sufficient.
17. How one CSV writer and one generalized plotting script keep five modules consistent while preserving module-specific metrics, and why no runtime timing is claimed.

### Suggested concise project explanation

> The Integrated OS Toolkit and Benchmarking Suite is a modular C application for Linux/POSIX that combines educational OS algorithm simulations with a consistent comparison workflow. Its scheduling module accepts one process workload and compares FCFS, non-preemptive SJF, non-preemptive Priority, and Round Robin; its page replacement module applies FIFO, LRU, and LFU to one reference string; its memory allocation module applies First Fit, Best Fit and Worst Fit to one block list; its disk scheduling module compares FCFS, SSTF and SCAN on one request queue and head position; its file allocation module compares contiguous, linked and indexed allocation on independent copies of one simulated block disk. All five comparison modules export module-specific metrics to CSV and charts through one shared reporting layer; deadlock avoidance, the POSIX demonstration and the IPC demonstration stay console-only because they have no alternative algorithms to compare. The POSIX demonstration uses real `fork()`, `exec()`, `wait()`/`waitpid()`, file and directory system calls, with all writes confined to a temporary file. Every reported value is a simulated result of the entered workload, not a measured runtime.

## 17. References and Further Reading

Use course-approved editions and add full publication years/edition numbers when preparing formal submissions.

- Abraham Silberschatz, Peter B. Galvin, and Greg Gagne, *Operating System Concepts*.
- Andrew S. Tanenbaum and Herbert Bos, *Modern Operating Systems*.
- Remzi H. Arpaci-Dusseau and Andrea C. Arpaci-Dusseau, *Operating Systems: Three Easy Pieces*.
- IEEE POSIX / IEEE Std 1003.1 documentation.
- Linux man-pages project for Linux/POSIX system-call behaviour.
- Project repository: <https://github.com/adityaDev-2005/Integrated-OS-Toolkit>

---

## Maintenance Note

This file is the project knowledge base, not proof that every listed future feature has been completed. When updating it, change the status table, repository tree, setup instructions, test data and roadmap based on the actual code. Record implementation claims only after the corresponding code builds and a test confirms the expected behaviour.
