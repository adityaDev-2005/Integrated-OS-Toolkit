# Integrated OS Toolkit and Benchmarking Suite — Project Knowledge Base

> **Purpose of this file:** A living technical reference for the project. It records the project's goals, current implementation, design decisions, run instructions, known limitations, and remaining work so that team members can continue development consistently.
>
> **Status snapshot:** Review 1 / initial implementation milestone. Update the status sections whenever modules are completed or the architecture changes.

## 1. Project Overview

**Project name:** Integrated OS Toolkit and Benchmarking Suite  
**Short description:** A modular, interactive Operating Systems toolkit written in C for Linux/POSIX. It is intended to combine OS algorithm simulations, practical OS programming demonstrations, and automatic performance-result reporting in a single command-line application.

**Primary repository:** <https://github.com/adityaDev-2005/Integrated-OS-Toolkit>  
**Primary language:** C11  
**Build tools:** GCC and Make  
**Graphing:** Python 3 and Matplotlib  
**Target environment:** Linux, including Linux Mint/Ubuntu; other POSIX-like environments may need minor adjustments.

### The core idea

Rather than maintaining a separate program for each OS algorithm, the toolkit provides one menu-driven application. For the current CPU scheduling module, the user enters one process workload and the program evaluates four scheduling algorithms using copies of that same workload. It displays process-level metrics and summaries, writes the comparison to CSV, and triggers Python/Matplotlib to generate a graph automatically.

The project does **not** propose new CPU-scheduling algorithms. Its engineering contribution is the integration of existing algorithms, shared inputs, consistent metric calculation, repeatable comparison, export, visualization, and the planned combination of simulations with real Linux/POSIX demonstrations.

## 2. Problem Statement and Motivation

Operating Systems courses introduce algorithms such as CPU scheduling, page replacement, memory allocation, deadlock avoidance, disk scheduling, and file allocation. If each topic is implemented as an isolated program, users may need to re-enter inputs repeatedly, calculate metrics manually, and prepare graphs separately. That process makes comparisons less convenient and creates opportunities for inconsistent inputs or arithmetic errors.

The proposed toolkit addresses this through:

- A single command-line interface and modular source structure.
- Reusable input and reporting patterns.
- Common workloads for comparing algorithms within a module.
- Automatic calculation of applicable performance metrics.
- CSV output that can be inspected or processed by other tools.
- Automatic graph generation from the saved results.
- Planned POSIX programming demonstrations to connect OS theory to actual system APIs.

Existing educational simulators may already provide some of these capabilities. The project's distinction is the specific combination and implementation in this modular C toolkit, not a claim that all individual features are unique.

## 3. Objectives

1. Build one modular C application for multiple Operating Systems concepts.
2. Implement classic OS algorithms and make their behaviour observable.
3. Compare algorithms using identical inputs within each module.
4. Calculate appropriate per-process and aggregate metrics automatically.
5. Display execution timelines or other useful intermediate results.
6. Export comparison data to CSV and generate plots automatically.
7. Demonstrate Linux/POSIX programming through process APIs, IPC, threads, and synchronization in later modules.
8. Keep the code modular so new algorithms and modules can be added without rewriting the complete application.
9. Document test cases, assumptions, setup, and usage for project demonstration and future maintenance.

## 4. Scope and Status at a Glance

The statuses below distinguish the working initial milestone from the intended full toolkit. Do not describe a planned module as complete until its implementation and tests have been verified.

| Module / capability | Status at Review 1 | Notes |
|---|---|---|
| Main CLI and menu | Implemented | Text-based menu dispatches to CPU scheduling; other menu items identify planned work. |
| FCFS CPU scheduling | Implemented | Non-preemptive, ordered by arrival time. |
| SJF CPU scheduling | Implemented | Non-preemptive; shortest burst among processes that have arrived. |
| Priority CPU scheduling | Implemented | Non-preemptive; lower numeric value means higher priority. |
| Round Robin CPU scheduling | Implemented | Preemptive time-slicing using a circular ready queue and user-provided quantum. |
| CPU scheduling metrics and timelines | Implemented | Per-process CT, TAT, WT, RT, plus averages and Gantt-style timeline. |
| CPU comparison summary | Implemented | Four algorithms use copies of the same workload. |
| CPU scheduling CSV export | Implemented | Writes `data/results/cpu_scheduling.csv` locally. |
| Automatic graph generation | Implemented | C program triggers the Python plotting script; PNG saved locally. |
| Page replacement | Planned / not yet integrated | FIFO, LRU, LFU; count page faults. |
| Memory allocation | Planned | First Fit, Best Fit, Worst Fit. |
| Deadlock avoidance | Planned | Banker's algorithm and safe-sequence analysis. |
| Disk scheduling | Planned | FCFS, SSTF, SCAN. |
| File allocation | Planned | Contiguous, linked, indexed allocation. |
| POSIX process demonstration | Planned | `fork()`, `exec()`, `wait()`, process IDs, and file/system calls. |
| IPC and producer-consumer | Planned | Pipes/shared memory plus pthreads and semaphores. |
| General-purpose benchmarking layer | Planned / not yet integrated | Shared logger and benchmark driver for future modules. |
| Real wall-clock algorithm runtime benchmarking | Not currently implemented | Current CPU comparisons are simulated scheduling metrics, not algorithm execution-time measurements. |

## 5. Current CPU Scheduling Workflow

1. The user launches the application and selects **Run CPU Scheduling Comparison**.
2. The application asks for the number of processes (currently 1–100).
3. For every process, it asks for arrival time, burst time, and priority.
4. It asks for the Round Robin time quantum.
5. Input is validated; invalid values are re-prompted.
6. Processes are sorted by arrival time while preserving input order for arrival-time ties.
7. The workload is copied so each algorithm begins with the same process data and fresh result fields.
8. The program runs FCFS, non-preemptive SJF, non-preemptive Priority, and Round Robin.
9. For each algorithm, it prints a Gantt-style timeline and a process table.
10. It calculates and prints average waiting time, average turnaround time, and average response time.
11. It prints a four-algorithm comparison table.
12. It saves the aggregate comparison to CSV.
13. It invokes `scripts/plot_results.py`, which reads the CSV and writes the PNG graph.
14. The C program attempts to open the image with `xdg-open` on a Linux desktop.
15. Control returns to the main menu.

### Current input constraints

| Input | Current validation range |
|---|---:|
| Number of processes | 1–100 |
| Arrival time | 0–1,000,000 |
| Burst time | 1–1,000 |
| Priority | 1–1,000,000 |
| Round Robin quantum | 1–1,000 |

Priority convention: **a smaller number represents a higher priority**.

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

No database is used at this stage. A flat CSV file is sufficient for the current small workload and can be read by Python, a spreadsheet program, or other analysis tools.

### CSV output

**Path:** `data/results/cpu_scheduling.csv`

Current columns:

- `algorithm`
- `avg_waiting_time`
- `avg_turnaround_time`
- `avg_response_time`
- `quantum`
- `process_count`

The file contains one aggregate result row for each of the four algorithms. Values are stored to two decimal places. The Round Robin quantum and process count are recorded for context.

### Graph output

**Script:** `scripts/plot_results.py`  
**Output:** `data/results/cpu_scheduling_comparison.png`

The script reads the CSV and generates three side-by-side bar charts:

1. Average Waiting Time.
2. Average Turnaround Time.
3. Average Response Time.

Every plot compares FCFS, SJF, Priority, and Round Robin for the same user-supplied workload. Lower values are preferable for each individual metric, but the algorithm best suited to a use case depends on the metric and workload.

### Automatic C-to-Python integration

After successfully writing the CSV, the C program invokes the plotting script. The current launch flow expects the application to be started from the **project root** so relative paths such as `.venv/bin/python`, `scripts/plot_results.py`, and `data/results/` resolve correctly. It may try system Python if the virtual-environment invocation fails, then attempts to open the PNG using `xdg-open`.

If graph generation fails, check that:

- Matplotlib is installed in `.venv` or the system Python environment used by the program.
- The CSV was created and contains all four algorithm rows.
- The program was launched from the project root.
- The plotting script exists and can be run directly for debugging.
- A graphical desktop session is available if automatic image opening is expected.

## 7. Architecture

The intended architecture has four layers:

1. **User interface layer:** `src/main.c`, the menu, and validated input handling.
2. **Algorithm/module layer:** independent modules for CPU scheduling and future OS topics.
3. **Metrics and data layer:** metric calculation and CSV output, eventually generalized through a shared benchmarking module.
4. **Visualization layer:** Python/Matplotlib reads generated CSV output and produces figures.

The current working execution path is primarily the main CLI, CPU scheduling implementation, CSV output inside the scheduling workflow, and plotting script. The existing benchmark and other module files should not be described as integrated features until the Makefile/build path and runtime behaviour actually use them.

## 8. Repository Structure

The project structure has included the following files. Some non-CPU source files are placeholders or planned modules; verify their status before treating them as complete features.

```text
Integrated-OS-Toolkit/
├── Makefile
├── README.md
├── include/
│   ├── benchmark.h
│   ├── common.h
│   ├── paging.h
│   └── scheduling.h
├── src/
│   ├── main.c
│   ├── scheduling/
│   │   ├── scheduling.c
│   │   └── scheduling_old.c       # local/untracked backup may exist; not required by the build
│   ├── paging/
│   │   └── paging.c               # planned/not yet integrated
│   ├── posix/
│   │   ├── process_demo.c         # planned/not yet integrated
│   │   └── producer_consumer.c    # planned/not yet integrated
│   └── benchmark/
│       ├── benchmark.c            # future shared benchmarking module
│       └── csv_logger.c           # future/generalized CSV logger
├── scripts/
│   └── plot_results.py
├── tests/
│   └── test_cases.md
├── data/
│   └── results/                   # generated locally at runtime
└── build/
    └── os_toolkit                # compiled locally; do not commit
```

### Build configuration

At the current milestone, the Makefile builds the main program and scheduling implementation, with the scheduling header as a dependency. The compiler flags include:

- `-Wall`
- `-Wextra`
- `-Wpedantic`
- `-std=c11`
- `-Iinclude`

The source list should be updated when future modules are integrated.

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

### 5. Run the application from the project root

```bash
./build/os_toolkit
```

Select option `1` for CPU Scheduling Comparison, then enter the process count, arrival time, burst time and priority for each process, followed by the Round Robin quantum.

### 6. Check locally generated outputs

After the comparison finishes, the application should create these files:

```text
data/results/cpu_scheduling.csv
data/results/cpu_scheduling_comparison.png
```

To list them:

```bash
ls -lh data/results/
```

To inspect the CSV:

```bash
cat data/results/cpu_scheduling.csv
```

The application should attempt to open the graph automatically. For manual troubleshooting only, run:

```bash
.venv/bin/python scripts/plot_results.py
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

## 11. Current Limitations and Design Decisions

1. **Simulated metrics only:** The current graphs compare waiting, turnaround, and response times calculated by the simulation. The program does not currently measure the C algorithm's real execution time or real OS context-switch overhead.
2. **Non-preemptive SJF and Priority:** Preemptive SJF/SRTF and preemptive Priority are not part of the current implementation.
3. **Priority direction:** A smaller integer denotes higher priority. This must be stated in the interface and report.
4. **Tie-breaking:** Deterministic tie rules are used to keep results repeatable; document any future changes to those rules.
5. **Paths:** The automatic plotting launch currently expects execution from the project root. Making paths independent of the current working directory is a possible improvement.
6. **Desktop dependency:** `xdg-open` is specific to Linux desktop environments. The PNG can still be generated on a headless machine, but automatic opening may not be available.
7. **External Python dependency:** The plotting step requires Python and Matplotlib. The C scheduling results can be computed independently, but the graph step depends on the plotting environment.
8. **Generated artifacts:** The CSV, PNG, executable, and virtual environment are runtime/build outputs and should normally remain untracked unless the team deliberately wants to publish sample artifacts.
9. **No database:** Data is currently stored as a simple CSV, which is appropriate for the current scope.
10. **Planned modules are not yet integrated:** Files existing in `src/` or `include/` do not automatically mean those modules are built, tested, or available through the menu.

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
- [ ] Add or verify explicit unit/regression tests for edge cases.
- [ ] Ensure graph generation reports failures clearly and works from a fresh clone.
- [ ] Confirm that README setup instructions match the final code.

### Phase B — Page replacement

Implement FIFO, LRU and LFU using one shared page-reference string and frame count. Report page hits and faults and compare all policies on identical input. Define the LFU tie-breaking rule and decide whether frames begin empty for every algorithm (they normally should for a fair independent comparison). Consider visualizing fault counts after the CSV/plot workflow is generalized.

### Phase C — Memory allocation

Implement First Fit, Best Fit and Worst Fit with common memory-block sizes and process/request sizes. Display block allocation, internal/external fragmentation metrics if defined, and failures to allocate. Clearly define whether blocks are split and how remaining space is represented.

### Phase D — POSIX process management

Build demonstrations using actual Linux/POSIX APIs, such as:

- `fork()` to create a child process.
- `exec()` family functions to replace a process image.
- `wait()` / `waitpid()` to collect child status.
- `getpid()` / `getppid()` for process identification.
- File APIs such as `open()`, `read()`, `write()`, `close()`, `lseek()`, `stat()`, and `fcntl()`.
- Directory operations using `opendir()` and `readdir()`.

Keep system-call demonstrations clearly separate from simulations of OS algorithms.

### Phase E — IPC and synchronization

Implement and explain:

- Pipes for inter-process communication.
- Shared memory if included in the final scope.
- Pthreads for multithreading.
- POSIX semaphores and a producer-consumer example.
- Correct synchronization, cleanup, and error handling.

Document the difference between a demonstration and a production-ready concurrent system.

### Phase F — Deadlock avoidance

Implement Banker's algorithm with allocation, maximum, need and available structures. Report whether the system is safe and print a safe sequence when one exists. Validate dimensions and reject impossible or invalid input.

### Phase G — Disk scheduling

Implement FCFS, SSTF and SCAN. Use one common request queue and initial head position, document disk-cylinder bounds and SCAN direction, calculate total head movement, and compare the results.

### Phase H — File allocation

Demonstrate contiguous, linked and indexed allocation using an in-memory or clearly specified simulated disk-block model. Explain the assumptions and show allocation/retrieval behaviour. Do not imply that the program modifies a real filesystem unless it actually does.

### Phase I — Generalized benchmarking and final integration

- Extract common logging into the planned `src/benchmark/` module.
- Define a consistent CSV schema for each type of module while retaining module-specific metrics.
- Add reusable charting patterns, labels, and error handling.
- Add tests and documented sample inputs for every implemented module.
- Update menu, Makefile, headers, README and this knowledge base as features become functional.
- Conduct an integration test from a clean clone.
- Prepare a final demonstration and report that distinguish implemented features from stretch goals.

## 13. Potential Future Improvements

These are optional enhancements, not current capabilities:

- Support command-line flags or batch input for reproducible experiments.
- Use a configuration file or input file for large test cases.
- Add a test runner and known expected outputs for regression testing.
- Let users compare results from multiple workloads.
- Make output paths independent of the working directory.
- Add a graceful headless mode that generates a graph without trying to open a desktop viewer.
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
7. How the C program writes aggregate metrics to CSV and invokes Python/Matplotlib.
8. Why the graph measures simulated scheduling metrics rather than real implementation runtime.
9. How the architecture can be extended to page replacement, memory management and POSIX demonstrations.
10. Which modules are implemented today versus planned for future phases.

### Suggested concise project explanation

> The Integrated OS Toolkit and Benchmarking Suite is a modular C application for Linux/POSIX that combines educational OS algorithm simulations with a consistent comparison workflow. Its current module accepts one process workload and compares FCFS, non-preemptive SJF, non-preemptive Priority, and Round Robin. It computes scheduling metrics, shows timelines, exports aggregate results to CSV, and automatically generates comparison graphs. The architecture is intended to grow to page replacement, memory allocation, deadlock avoidance, disk/file allocation, and practical POSIX process and synchronization demonstrations.

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
