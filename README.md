# Integrated OS Toolkit and Benchmarking Suite

A modular Operating Systems project implemented in C for Linux/POSIX.

## Quick start

Requirements: a Linux/POSIX environment (native Linux, WSL or a container)
with GCC and Make. Python 3 with Matplotlib is optional and only needed for the
comparison charts; without it the CSV files are still written.

```bash
sudo apt install build-essential        # once, on Debian/Ubuntu
make
./build/os_toolkit
```

The build produces `build/os_toolkit`. The binary may be started from any
directory: generated files always go to `data/results/` inside the project.
The project root is taken from `OS_TOOLKIT_ROOT` when set, otherwise from the
location of the executable.

Everything is driven by a numbered menu. Every input is validated: invalid
values are rejected and re-prompted, and End of input exits the program like
menu choice `10`.

## Menu at a glance

| Option | Module | What it asks for | Result |
|---|---|---|---|
| 1 | CPU scheduling (FCFS, SJF, Priority, Round Robin) | process count, priority ordering, arrival/burst/priority per process, RR quantum | console report + CSV + chart |
| 2 | Page replacement (FIFO, LRU, LFU) | frame count, page reference string | console report + CSV + chart |
| 3 | Memory allocation (First, Best, Worst Fit) | block sizes, request sizes | console report + CSV + chart |
| 4 | POSIX process and file demonstration | nothing, it runs immediately | live `fork`/`exec`/file/`readdir` output |
| 5 | IPC and synchronization demonstration | buffer size, item count, thread counts, or the pipe submenu | live thread, semaphore and pipe output |
| 6 | Deadlock avoidance (Banker's algorithm) | Allocation, Maximum and Available values | safety verdict + safe sequence |
| 7 | Disk scheduling (FCFS, SSTF, SCAN) | cylinder range, initial head, request queue | console report + CSV + chart |
| 8 | File allocation (contiguous, linked, indexed) | disk blocks, block size, file operations | console report + CSV + chart |
| 9 | About | nothing | project summary |
| 10 | Exit | nothing | closes the program |

## Example session

A two-process CPU scheduling run with the default priority convention:

```text
Enter choice: 1
Number of processes (1-100): 2
Priority ordering:
  1. Smaller numbers mean higher priority
  2. Larger numbers mean higher priority
Select priority ordering (1-2): 1
Enter arrival time, burst time, and priority for each process.
Smaller priority numbers mean higher priority.
P1 arrival time: 0
P1 burst time: 10
P1 priority: 2
P2 arrival time: 0
P2 burst time: 2
P2 priority: 9
Round Robin time quantum (1-1000): 2
```

With ordering `1` the Priority algorithm runs P1 first (priority 2). With
ordering `2` it runs P2 first (priority 9), so only the Priority row of the
comparison changes; FCFS, SJF and Round Robin ignore priority values.

## Objectives

- Simulate classical operating-system algorithms.
- Demonstrate Linux/POSIX system calls and inter-process communication.
- Implement concurrent workloads using pthreads and semaphores.
- Benchmark algorithms and compare their results using CSV files and charts.

## Status

| Area | Status |
|---|---|
| CLI menu | Implemented |
| FCFS CPU scheduling | Implemented |
| SJF CPU scheduling (non-preemptive) | Implemented |
| Priority CPU scheduling (non-preemptive) | Implemented |
| Round Robin CPU scheduling | Implemented |
| CSV export and comparison chart | Implemented |
| Regression test suite (`make test`) | Implemented |
| Page replacement (FIFO/LRU/LFU) | Implemented |
| Memory allocation (First/Best/Worst Fit) | Implemented |
| POSIX process and file demonstration | Implemented |
| IPC and synchronization demonstration | Implemented |
| Deadlock avoidance (Banker's algorithm) | Implemented |
| Disk scheduling (FCFS/SSTF/SCAN) | Implemented |
| File allocation (contiguous/linked/indexed) | Implemented |
| CSV and chart reporting (five comparison modules) | Implemented |
| Runtime (wall-clock) benchmarking | Not implemented (out of scope) |

See `PLAN.md` for the milestone breakdown and `PROJECT_KNOWLEDGE_BASE.md` for
the full technical reference.

## Scope and authenticity

This repository implements only the modules, algorithms and demonstrations named
in `PROJECT_KNOWLEDGE_BASE.md`; it does not add unrelated OS features.

- CPU scheduling, page replacement, memory allocation, deadlock avoidance, disk
  scheduling and file allocation are simulations of OS behavior. Their metrics
  are computed from the entered workload, not measured from a real kernel. The
  file allocation module works on an in-memory block disk and never reads or
  modifies a real filesystem.
- The POSIX demonstration (menu option `4`) executes real Linux system calls:
  `fork()`, `exec()`, `wait()`/`waitpid()`, process-ID calls, file APIs and
  directory APIs. Its PIDs, statuses and file results are actual runtime values.
- The IPC demonstration (menu option `5`) runs real pthreads, real POSIX
  semaphores and a real `pipe()` between a parent and a forked child. Its item
  counts, buffer occupancy and received bytes are actual runtime values.
- No simulated result is presented as measured runtime.

## Technologies

- C11 and GCC
- Linux/POSIX APIs
- POSIX threads and semaphores
- Make
- Python 3 and Matplotlib for visualization

## Results and charts

Every algorithm-comparison module writes one CSV and one PNG under
`data/results/`:

| Menu option | CSV | Chart |
|---|---|---|
| 1 CPU scheduling | `cpu_scheduling.csv` | `cpu_scheduling_comparison.png` (3 panels, unchanged) |
| 2 Page replacement | `page_replacement.csv` | `page_replacement_comparison.png` (hits vs faults) |
| 3 Memory allocation | `memory_allocation.csv` | `memory_allocation_comparison.png` (2x2 panels) |
| 7 Disk scheduling | `disk_scheduling.csv` | `disk_scheduling_comparison.png` (head movement) |
| 8 File allocation | `file_allocation.csv` | `file_allocation_comparison.png` (2x2 panels) |

Shared CSV convention: the first column is `algorithm`, the metric columns are
snake_case, workload-context columns follow the metrics, floating-point values
use two decimals, and each file has one row per algorithm. There is no timestamp
or runtime column: every value is a simulated result of the entered workload,
not the measured execution time of the C code.

Banker's algorithm (option 6), the POSIX demonstration (option 4) and the IPC
demonstration (option 5) are not algorithm comparisons, so they stay
console-only and produce no CSV or chart.

`scripts/plot_results.py` accepts the module name, validates the CSV and writes
the PNG. A failed CSV write prints
`CSV export failed; the algorithm results above are still valid.` and skips the
plot. A failed plot prints `Could not generate the graph.` and states that the
CSV and the algorithm results are still valid.

## CPU scheduling

Select menu option `1`. Enter the number of processes, choose the priority
ordering (smaller numbers first or larger numbers first), then enter arrival
time, burst time and priority for each process, then the Round Robin quantum.
The ordering choice affects only the Priority algorithm; FCFS, SJF and Round
Robin do not use priority values.

The program runs FCFS, non-preemptive SJF, non-preemptive Priority and Round
Robin on the same workload, prints Gantt-style timelines and per-process
metrics, then writes:

- `data/results/cpu_scheduling.csv`
- `data/results/cpu_scheduling_comparison.png`

## Page replacement

Select menu option `2`. Enter the number of frames (1–100), the number of page
references (1–1000), then one page number (0–999) per reference.

FIFO, LRU and LFU each run over the same reference string starting from empty
frames. The program reports page hits and page faults for all three policies in
one comparison table. LFU evicts the smallest access count, breaking ties by the
earliest loaded page.

CSV export and chart: the module writes `data/results/page_replacement.csv` and
`data/results/page_replacement_comparison.png` with hits and faults per policy.

## Memory allocation

Select menu option `3`. Enter the number of memory blocks (1–100), the size of
each block (1–1000000), the number of allocation requests (1–100), then the
size of each request (1–1000000).

First Fit, Best Fit and Worst Fit each start from a fresh copy of the same
block list and process the same requests. A request is granted exactly its
requested size from the chosen block, and the remaining free space stays
available for later requests. Because allocation is an exact split, internal
fragmentation (allocated size minus requested size) is zero. External
fragmentation is reported as the largest free block. Requests that no block can
satisfy are reported as failed and do not consume space.

The report shows the block chosen for every request, the remaining free space
of every block and a summary of allocations, failures and fragmentation for all
three policies. The module also writes `data/results/memory_allocation.csv` and
`data/results/memory_allocation_comparison.png`.

## POSIX process and file demonstration

Select menu option `4`. The demonstration runs immediately and needs no further
input. It performs real system calls in this order:

- Prints the parent `getpid()` and `getppid()`.
- Creates two children with `fork()`; one is reaped with `waitpid()` and the
  other with `wait()`, and both exit statuses are reported.
- Replaces a child image with `execlp("echo", ...)` and reports the reaped
  status.
- Attempts `execl()` on a missing path, reports the real `ENOENT` error and
  confirms the child exits with status 127.
- Creates one temporary file with `mkstemp()`, then demonstrates `open()`,
  `write()`, `fcntl()`, `lseek()`, `read()`, `stat()` and `close()` on it. The
  file contains `hello world`; it is unlinked before the menu returns.
- Lists up to five entries from `/tmp` with `opendir()` and `readdir()`, then
  calls `closedir()`.

All writes are confined to the temporary file. No project file is modified, and
no pipes, shared memory or other IPC are used in this module.

## IPC and synchronization

Select menu option `5`, then choose a demonstration. Both use real kernel
concurrency primitives; nothing is simulated.

### Producer-consumer (pthreads + POSIX semaphores)

Enter the bounded buffer size (1–100), the number of items (1–10000), and the
number of producer and consumer threads (1–8 each).

- Producers take unique item identifiers and insert them into a bounded ring
  buffer. An `empty` semaphore counts free slots and a `full` semaphore counts
  buffered items; a mutex protects the ring itself.
- Consumers reserve one item before waiting, so every consumer terminates once
  the item count is exhausted without needing a fixed quota.
- After all threads are joined the module verifies that produced equals consumed,
  that every identifier appears exactly once, that no invalid identifier was
  read, and that the ring is empty. It also reports the peak occupancy, which can
  never exceed the configured capacity.
- Every thread is joined, and the semaphores, mutexes and heap buffers are
  released on success and on every error path.

### Pipe (fork + pipe)

The child closes the unused read end, writes a fixed message, closes the write
end and exits. The parent closes the unused write end, reads until `read()`
returns 0, reports the byte count and the received data, then reaps the child
with `waitpid()` and checks its exit status. Every `pipe`, `fork`, `read`,
`write`, `close` and `waitpid` result is checked and interrupted calls are
retried on `EINTR`.

Both demonstrations are teaching examples, not production concurrency
frameworks: they provide no cancellation, timeout, back-pressure policy or work
stealing. Shared memory is excluded from this project by scope decision Q1, so
no shared-memory mechanism is used.

## Deadlock avoidance (Banker's algorithm)

Select menu option `6`. Enter the number of processes and resource types (1–20
each), the Allocation matrix, the Maximum matrix and the Available vector. Every
value is validated: negative, non-numeric and out-of-range input is rejected and
re-prompted.

The program derives `Need = Maximum - Allocation`, prints all three matrices and
the Available vector, and runs the safety algorithm with `Work` starting at
`Available`. In each step the lowest-numbered process whose Need fits the current
Work is marked finishable and its Allocation is added to Work.

- If every process finishes, the state is safe and a valid safe sequence is
  printed. The lowest-numbered ready process is always chosen first, so one
  deterministic sequence is reported when the state has several.
- If some processes remain, the state is unsafe and the unfinished processes are
  listed. An unsafe state is not reported as a deadlock: it means no safe
  completion order is guaranteed for that state.
- An Allocation entry larger than the same process's Maximum is inconsistent, so
  the state is rejected before any analysis.

The matrices describe a simulated resource-allocation state; no kernel resource
table is read. Request vectors and resource release are outside this module.
Because there are no alternative algorithms to compare, this module stays
console-only and produces no CSV or chart.

## Disk scheduling

Select menu option `7`. Enter the minimum cylinder (0–9998), the maximum
cylinder (larger than the minimum, up to 9999), the initial head position and
the number of requests (1–200), then one cylinder number per request. Every
value is validated against the configured bounds.

FCFS, SSTF and SCAN all read the same request queue and the same initial head
position, and each reports its service order and total head movement. The three
movement totals are then repeated in one console comparison table:

- **FCFS** serves the queue in input order.
- **SSTF** always serves the nearest pending request. Ties are deterministic:
  equal distance goes to the lower cylinder number, and equal cylinder numbers
  go to the earlier request in the queue.
- **SCAN** starts toward the maximum cylinder and serves every pending request
  on the way up. The arm always reaches that boundary before reversing, and
  that travel counts toward the total even when no request lies there. The
downward sweep serves the remaining requests in descending order and stops at
the last one. This is conventional SCAN, not LOOK.

CSV export and chart: the module writes `data/results/disk_scheduling.csv` and
`data/results/disk_scheduling_comparison.png` with total head movement per
policy.

## File allocation

Select menu option `8`. Enter the number of simulated disk blocks (1–128), the
block size in bytes (8–4096) and the number of file operations (1–64). Each
operation is an allocation, a deletion or a retrieval identified by a file ID
(1–32); an allocation also takes a file size in bytes (1–1,000,000). Every value
is validated and re-prompted.

Contiguous, linked and indexed allocation each run on an independent copy of
the same empty disk and receive the same operation sequence:

- **Contiguous** needs `ceil(size / block_size)` data blocks and takes the first
  free run that fits.
- **Linked** reserves a 4-byte next pointer in every data block, so each block
  carries `block_size - 4` bytes of file data and the file needs
  `ceil(size / (block_size - 4))` blocks, taken in ascending free-block order.
- **Indexed** needs `ceil(size / block_size)` data blocks plus one index block.
  The index block holds `block_size / 4` 4-byte pointers; a file that needs more
  data blocks than that is rejected. The index block is allocated first.

Allocation is atomic: capacity and free-space checks run before the disk
changes, so a failed request leaves the volume untouched and reports the reason
(duplicate file ID, insufficient free blocks, no contiguous run, index capacity,
missing file). Deletion frees all data and index blocks, and retrieval prints
the stored mapping.

Per method the report shows every operation, `Block usage` with the used and
free block lists, a summary of allocations, deletions and retrievals, and
`Space: requested ... allocated ... wasted ...; metadata blocks ...`. Wasted
space is `allocated block bytes - requested bytes` over the active files, so it
includes last-block slack, link-pointer slots and index blocks; index blocks
are also reported as the metadata-block count.

All three methods start from the same empty disk and the same block geometry,
and the module states in its banner that no real filesystem is read or
modified. It writes `data/results/file_allocation.csv` and
`data/results/file_allocation_comparison.png` covering block usage, operation
counters, bytes and waste.

## Tests

```bash
make test
```

`tests/run_tests.sh` runs eight scheduling workloads from a temporary directory
and compares the generated CSV with fixtures in `tests/cases/`. It also runs
five page-replacement workloads and compares the reported hits and faults, five
memory-allocation workloads and compares the full allocation report, one POSIX
workload that checks required markers, the parent/child PID relationship,
removal of the temporary file and an empty working directory, four IPC
workloads that check the stable producer-consumer and pipe markers, the rejected
input paths and an edge configuration, five deadlock-avoidance workloads that
check a safe state, an unsafe state, a state with several valid sequences, a
rejected allocation and an all-zero state, six disk-scheduling workloads
that compare the full policy report for mixed requests, one-sided requests,
cylinder boundaries, duplicate requests, SSTF distance ties and rejected
cylinder values, and seven file-allocation workloads that compare the full
report for mixed allocation, exhausted space, deletion and reuse, contiguous
fragmentation, indexed capacity limits, the minimum disk and rejected input.

Ten integration checks follow the fixtures:

- Five reporting checks rerun one workload per comparison module under an
  isolated `OS_TOOLKIT_ROOT` and compare the generated CSV with
  `reporting_*.expected.csv`; they also require the save and plot-skip messages
  and confirm that nothing is written into the current directory.
- One menu-dispatch check chains choices 1-8, About (9), an invalid choice (11)
  and Exit (10) through one process and requires every module banner and exactly
  five CSV writes.
- Two CSV-failure checks make `data` a regular file, so the export fails; the
  algorithm results must still appear and the run must exit 0.
- Two plot-failure checks use a root without `scripts/plot_results.py`; the CSV
  must be kept and the failure explained.

The suite runs 52 checks. The runner also checks the `OS_TOOLKIT_ROOT` override
and that nothing is written into the current directory. See `tests/test_cases.md`
for the matrix and expected values.

## Environment switches

| Variable | Effect |
|---|---|
| `OS_TOOLKIT_ROOT` | Overrides the project root used for output paths. |
| `OS_TOOLKIT_NO_PLOT=1` | Skips plot generation; the CSV is still written. |
| `OS_TOOLKIT_NO_OPEN=1` | Generates the plot but does not open it. |

When neither `DISPLAY` nor `WAYLAND_DISPLAY` is set, the image is never opened.

## Setup

1. Install the toolchain:

   ```bash
   sudo apt update
   sudo apt install build-essential python3 python3-venv
   ```

2. Optional: create a virtual environment with Matplotlib. Without it the
   program falls back to the system `python3`:

   ```bash
   python3 -m venv .venv
   .venv/bin/python -m pip install matplotlib
   ```

3. Build and run:

   ```bash
   make clean && make
   ./build/os_toolkit
   ```

If Matplotlib is unavailable the CSV is still produced and the program reports
that the graph could not be generated.

## Repository layout

```text
README.md  usage guide (this file)
Makefile   build and test targets
PLAN.md    milestone breakdown and progress log
PROJECT_KNOWLEDGE_BASE.md  full technical reference
AGENT_CONTEXT.md           short context summary for AI assistants
include/   public headers, one per module (banker.h, benchmark.h, common.h,
           disk.h, file_alloc.h, scheduling.h, paging.h, memory.h,
           process_demo.h, producer_consumer.h)
src/       main.c plus one directory per module (benchmark/, common/,
           scheduling/, deadlock/, disk/, file_alloc/, paging/, memory/,
           posix/)
scripts/   plot_results.py
tests/     test_cases.md, run_tests.sh and the cases/ fixtures
data/      generated CSV and PNG (git-ignored)
build/     compiled binary (git-ignored)
```

## Notes

Generated files do not need to be committed. Run the application again with a
new workload to regenerate them.

A clean tree (sources without `build/`, `data/results/`, `.git/` and `.venv/`)
builds with `make clean && make` and passes all 52 checks; `make test` leaves
only git-ignored output under `data/results/`.
