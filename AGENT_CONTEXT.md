# AGENT_CONTEXT - Continuity Notes

Purpose: this file lets any new session resume the work on Integrated OS Toolkit
without re-reading the whole repository. Read this file first, then `PLAN.md`.

- `PLAN.md` - what is planned, milestone checklist, progress log.
- `AGENT_CONTEXT.md` (this file) - verified facts about the repository, the
  environment, the commands, and the traps.
- `PROJECT_KNOWLEDGE_BASE.md` - the team's own project reference (source of the
  roadmap). Do not contradict it; update it when behavior changes.

## 1. Snapshot

| Item | Value |
|---|---|
| Repository | `D:\OS_project\Integrated-OS-Toolkit` (GitHub `adityaDev-2005/Integrated-OS-Toolkit`) |
| Branch / commit at audit | `main` / `db7a6c4` |
| Language | C11, POSIX (Linux target) |
| Build | GCC + Make |
| Plotting | Python 3 + Matplotlib (`scripts/plot_results.py`) |
| Working today | CPU scheduling (FCFS, non-preemptive SJF, non-preemptive Priority, Round Robin), page replacement (FIFO, LRU, LFU), memory allocation (First, Best, Worst Fit), Banker's algorithm (deadlock avoidance), disk scheduling (FCFS, SSTF, SCAN), file allocation (contiguous, linked, indexed), the real POSIX process/file/directory demonstration, the real pthreads + POSIX semaphore producer-consumer and `fork()`/`pipe()` demonstration, and shared CSV/chart reporting for the five algorithm-comparison modules |
| Not implemented | Nothing within the approved scope; B1-B8 remain backlog |

## 2. Environment (verified)

The only toolchain that works on this machine is WSL.

| Environment | Status |
|---|---|
| WSL `NVIDIA-Workbench` | gcc 11.4, make, python3 with matplotlib - **use this one** |
| WSL `Ubuntu` | no gcc, no make; `sudo` needs a password - unusable |
| Native Windows MSYS2 GCC | build fails: MinGW `mkdir()` takes one argument |
| Windows `make` | not installed |

PowerShell is 5.1: `&&` is not a separator, use `;`.

## 3. Commands

Build (PowerShell):

```powershell
wsl -d NVIDIA-Workbench -e bash -c "cd /mnt/d/OS_project/Integrated-OS-Toolkit && make clean && make"
```

Run (PowerShell):

```powershell
wsl -d NVIDIA-Workbench -e bash -c "cd /mnt/d/OS_project/Integrated-OS-Toolkit && ./build/os_toolkit"
```

Run the regression suite:

```powershell
wsl -d NVIDIA-Workbench -e bash -c "cd /mnt/d/OS_project/Integrated-OS-Toolkit && make test"
```

Sample workload, one value per line (the program reads one value per prompt):
menu `1`, count `4`, priority ordering `1` (smaller numbers first), then
`0 5 2`, `1 3 1`, `2 8 4`, `3 6 3` as arrival / burst / priority per process,
then quantum `3`, then menu `10`:

```
1
4
1
0
5
2
1
3
1
2
8
4
3
6
3
3
10
```

Memory allocation sample (blocks 100 and 200, requests 200 and 100):

```
3
2
100
200
2
200
100
10
```

POSIX demonstration (menu 4 needs no further input, then exit with 10):

```
4
10
```

IPC demonstration: menu `5`, then submenu `1` (producer-consumer) or `2`
(pipe), then the demonstration inputs, then `3` to return and `10` to exit. A
producer-consumer run with buffer 5, 20 items, 2 producers and 2 consumers:

```
5
1
5
20
2
2
3
10
```

A pipe run needs no inputs beyond the submenu:

```
5
2
3
10
```

Deadlock avoidance: menu `6`, process count, resource count, the Allocation
matrix, the Maximum matrix, the Available vector, then `10`. This 3-process,
2-resource state is unsafe after P1 and P2 finish:

```
6
3
2
1
0
0
1
1
1
1
0
0
1
3
3
0
0
10
```

Disk scheduling: menu `7`, minimum cylinder, maximum cylinder, initial head
position, request count, one cylinder number per request, then the exit choice
`10`. Bounds 0-199, head 53 and requests 98 183 37 122 14 124 65 67:

```
7
0
199
53
8
98
183
37
122
14
124
65
67
10
```

File allocation: menu `8`, block count, block size, operation count, then one
operation per line group: type (`1` allocate, `2` delete, `3` retrieve), file ID
and, for an allocate only, the file size in bytes; then `10` to exit. This run
uses 10 blocks of 64 bytes:

```
8
10
64
4
1
1
100
3
1
2
1
2
2
2
10
```

## 4. File map

| Path | State |
|---|---|
| `src/main.c` | Working. Menu 1 (scheduling), 2 (paging), 3 (memory allocation), 4 (POSIX demonstration), 5 (IPC and synchronization), 6 (deadlock avoidance), 7 (disk scheduling), 8 (file allocation), 9 (about), 10 (exit). About names the reporting scope and states that benchmark metrics are simulated results. |
| `src/scheduling/scheduling.c` | Working. All four algorithms, Gantt segments, metrics, CSV, plot invocation. |
| `src/paging/paging.c` | Working (M1, reporting in M8). FIFO/LRU/LFU over one reference string, hits and faults, CSV and chart. |
| `src/memory/memory.c` | Working (M2, reporting in M8). First/Best/Worst Fit over one block list, exact splitting, fragmentation and failures, CSV and chart. |
| `src/posix/process_demo.c` | Working (M3). Real fork/exec/wait/waitpid, PID reporting, temporary-file APIs, directory APIs, cleanup. |
| `src/posix/producer_consumer.c` | Working (M4). pthreads + POSIX semaphore bounded-buffer producer-consumer with integrity check, plus the `fork()`/`pipe()` demonstration. |
| `src/deadlock/banker.c` | Working (M5). Allocation/Maximum/Available input, Need derivation, rejection of an allocation above the maximum, safety algorithm and safe/unsafe report. |
| `src/disk/disk.c` | Working (M6, reporting in M8). FCFS, SSTF and SCAN over one shared request queue, head and cylinder bounds; head path and total movement per policy, a console comparison table and a module CSV/chart. |
| `src/file_alloc/file_alloc.c` | Working (M7, reporting in M8). Contiguous, linked and indexed allocation over independent copies of one simulated disk; allocate/delete/retrieve, block usage, failures, wasted space and metadata blocks per method, CSV and chart. |
| `src/common/common.c` | Project-root path resolution and `read_integer()` input validation. |
| `include/scheduling.h` | `Process` struct and `run_cpu_analyzer()`; `MAX_PROCESSES 100`. |
| `include/paging.h` | `run_page_replacement()`, `MAX_PAGE_REFERENCES 1000`, `MAX_FRAMES 100`. |
| `include/memory.h` | `run_memory_allocation()`, `MAX_MEMORY_BLOCKS 100`, `MAX_MEMORY_REQUESTS 100`, `MAX_MEMORY_SIZE 1000000`. |
| `include/process_demo.h` | `run_posix_demo()`; no input limits because the demo is self-driven. |
| `include/producer_consumer.h` | `run_ipc_demo()`; the submenu owns the input limits. |
| `include/banker.h` | `run_deadlock_avoidance()`; `MAX_BANKER_PROCESSES 20`, `MAX_BANKER_RESOURCES 20`, `MAX_BANKER_VALUE 999`. |
| `include/disk.h` | `run_disk_scheduling()`; `MAX_DISK_REQUESTS 200`, `MAX_DISK_CYLINDER 9999`. |
| `include/file_alloc.h` | `run_file_allocation()`; `MAX_FILE_BLOCKS 128`, `MAX_SIM_FILES 32`, `MAX_FILE_OPERATIONS 64`, `MAX_FILE_BYTES 1000000`, block size 8-4096, `FILE_POINTER_BYTES 4`. |
| `include/common.h` | `project_path()` and `read_integer()`. |
| `scripts/plot_results.py` | Working. Dispatches on the module name, validates the CSV and writes one PNG. `cpu_scheduling` keeps the 3-panel layout; the other four modules use module-specific panels. Exits 1 with a clear message on bad input. |
| `tests/test_cases.md` | Regression matrix (scheduling, paging, memory allocation, POSIX, IPC, deadlock avoidance, disk scheduling, file allocation and M8 reporting/integration). |
| `tests/run_tests.sh` | Regression runner; `paging_*`, `memory_*`, `posix_*`, `ipc_*`, `banker_*`, `disk_*` and `file_*` fixtures are detected by name. Reporting, menu-dispatch, CSV-failure and plot-failure checks run after the fixture loop. |
| `tests/cases/*` | Input fixtures plus `*.expected.csv` or `*.expected.txt`; `reporting_*.expected.csv` freeze the four new module CSVs. |
| `src/benchmark/csv_logger.c` | Working (M8). Shared CSV writer: creates `data/results`, escapes commas, quotes and newlines, reports failures and deletes a partial file on a failed close. |
| `src/benchmark/benchmark.c` | Working (M8). Save/failure messages, Python plot launch (`.venv` then `python3`) and guarded `xdg-open`. |
| `include/benchmark.h` | The `BenchmarkCsv` handle and the reporting API used by the five comparison modules. |
| `data/results/` | Generated CSV/PNG, git-ignored. |

## 5. Verified reference behavior

Documented sample workload `(AT, BT, PR)`: P1 (0,5,2), P2 (1,3,1), P3 (2,8,4),
P4 (3,6,3), quantum 3.

| Algorithm | Avg WT | Avg TAT | Avg RT |
|---|---|---|---|
| FCFS | 5.75 | 11.25 | 5.75 |
| SJF | 5.25 | 10.75 | 5.25 |
| Priority | 5.25 | 10.75 | 5.25 |
| Round Robin | 8.50 | 14.00 | 3.00 |

Generated CSV for that workload:

```csv
algorithm,avg_waiting_time,avg_turnaround_time,avg_response_time,quantum,process_count
FCFS,5.75,11.25,5.75,3,4
SJF,5.25,10.75,5.25,3,4
Priority,5.25,10.75,5.25,3,4
Round Robin,8.50,14.00,3.00,3,4
```

These numbers are the regression oracle. If a change alters them, the change is
wrong unless the algorithm definition is intentionally being corrected (then
update `PROJECT_KNOWLEDGE_BASE.md` too).

Page replacement oracle (hits/faults):

| Frames | References | FIFO | LRU | LFU |
|---|---|---|---|---|
| 3 | 1 2 3 4 1 2 5 1 2 3 4 5 | 3/9 | 2/10 | 2/10 |
| 2 | 1 1 2 3 3 1 | 2/4 | 2/4 | 3/3 |

Every policy starts from empty frames. LFU counts accesses while the page is
resident and breaks ties by the earliest loaded page.

Memory allocation oracle (allocated requests / failed requests / largest free
block; internal fragmentation is 0 in every case):

| Blocks | Requests | First Fit | Best Fit | Worst Fit |
|---|---|---|---|---|
| 100 500 200 300 600 | 212 417 112 426 | 3 / 1 / 300 | 4 / 0 / 174 | 3 / 1 / 300 |
| 50 80 | 100 200 | 0 / 2 / 80 | 0 / 2 / 80 | 0 / 2 / 80 |
| 100 200 | 200 100 | 2 / 0 / 0 | 2 / 0 / 0 | 2 / 0 / 0 |
| 1000 | 300 300 300 | 3 / 0 / 100 | 3 / 0 / 100 | 3 / 0 / 100 |

Each policy starts from a fresh copy of the blocks. An allocation takes exactly
the requested size and leaves the remainder free. That exact split is why
internal fragmentation is zero; do not add alignment or a minimum unit to make
it nonzero.

POSIX demonstration (M3) - real calls, no simulated values:

| API group | Actual behavior |
|---|---|
| Process IDs | Parent, children and exec child print real `getpid()`/`getppid()` values; child PPID equals the parent PID |
| Waiting | First child exits 42 and is reaped by `waitpid()`; second exits 7 and is reaped by `wait()` |
| Exec success | Child calls `execlp("echo", ...)`; the image is replaced and the parent reaps status 0 |
| Exec failure | Child calls `execl()` on a missing path; `errno` is reported and the child exits 127 |
| File | `mkstemp()` creates `/tmp/os_toolkit_posix_XXXXXX`; `open`/`write`/`lseek`/`read`/`stat`/`fcntl`/`close` operate on it; content is `hello world`, size 11; file is unlinked |
| Directory | `opendir("/tmp")`, up to five `readdir()` entries, then `closedir()` |
| Cleanup | All children reaped, all descriptors closed, temporary file removed on every path |

Do not add IPC to the POSIX process/file module; pipes and the producer-consumer
demo belong to M4. Shared memory is excluded by decision Q1 (2026-10-22), so M4
uses pipes, pthreads and POSIX semaphores only.

IPC and synchronization (M4) - real threads, semaphores and a real pipe:

| Item | Actual behavior |
|---|---|
| Producer-consumer | `empty` counts free slots, `full` counts buffered items, a mutex protects the ring, a second mutex protects the counters; producers take unique IDs so the split between threads varies per run |
| Termination | Consumers reserve one item before waiting, so no consumer needs a fixed quota; the run ends when the item count is exhausted |
| Integrity line | Prints produced, consumed, duplicates, missing, invalid and the final ring state, plus peak occupancy; all must match the requested item count, and peak occupancy can never exceed the capacity |
| Pipe | Child closes the read end, writes 25 bytes (`hello from child via pipe`), closes the write end and exits 0; parent closes the write end, reads until `read()` returns 0, compares the data, closes the read end and reaps the child with `waitpid()` |
| Error handling | Every `pipe`, `fork`, `read`, `write`, `close`, `waitpid`, `sem_init`, `sem_wait`, `pthread_create` and `pthread_join` result is checked; `EINTR` is retried |
| Cleanup | Threads joined, semaphores and mutexes destroyed, heap buffers freed on success and on every early-return path |
| Abort path | A failed thread creation sets a stop flag and forwards semaphore tokens, so already-created threads are released and joined instead of hanging |

Deadlock avoidance (M5) - a simulated resource-allocation state:

| Item | Actual behavior |
|---|---|
| Input | Process count and resource count (1-20 each), the Allocation and Maximum matrices, then the Available vector; every value is validated and re-prompted by `read_integer()` |
| Need | `Need[i][j] = Maximum[i][j] - Allocation[i][j]`, printed as its own matrix |
| Rejection | An Allocation entry above the same process's Maximum prints the process, resource and both values, then `state: REJECTED`; no safety analysis runs |
| Safety | `Work` starts at `Available`; the lowest-numbered unfinished process whose Need fits finishes, and its Allocation is added to `Work` |
| Verdict | `state: SAFE` with `safe sequence: ...`, or `state: UNSAFE` with the unfinished processes and a note that unsafe is not the same as deadlocked |
| Determinism | Fully deterministic: identical input always yields the same sequence, because the tie-break is fixed |
| Scope | One static state per run; no request vectors, no release operations, no prevention or detection algorithms |

Disk scheduling (M6) - a simulated disk arm:

| Item | Actual behavior |
|---|---|
| Input | Minimum cylinder (0-9998), maximum cylinder (minimum+1 to 9999), initial head and request count (1-200), then one request per line; every value is validated against the configured bounds by `read_integer()` |
| Shared input | All three policies read the same immutable queue and head; each keeps its own service flags, path and movement total |
| FCFS | Serves requests in queue order; movement is the sum of absolute head deltas |
| SSTF tie-break | Nearest cylinder first; on equal distance the lower cylinder number wins; on equal cylinder numbers the earlier queue entry wins |
| SCAN | Conventional SCAN: fixed initial direction toward the maximum cylinder, every pending request on the way up is served, the arm always reaches the bound before reversing (that travel is counted), then the remaining requests are served downwards and the sweep stops at the last one |
| Explicitly not LOOK | The run to the upper boundary is included even when no request lies there; the downward sweep does not travel to the minimum bound unless a request sits there |
| Output | Per policy: `Head path: ...` starting at the initial head, `Total head movement: N cylinders`, then one comparison table and a note fixing the tie-break and the SCAN rule |
| Oracle fixtures | Classic mixed case FCFS 640 / SSTF 236 / SCAN 331; one side 40 / 40 / 89; boundaries 149 / 148 / 148; duplicates 30 / 30 / 50; SSTF tie 90 / 90 / 128 |

File allocation (M7) - a simulated block disk:

| Item | Actual behavior |
|---|---|
| Input | Block count (1-128), block size (8-4096 bytes), operation count (1-64), then per operation the type (1 allocate, 2 delete, 3 retrieve), file ID (1-32) and, for allocation, the size in bytes (1-1000000); every value is validated and re-prompted |
| Shared setup | Each method runs on its own copy of the same empty disk with the same operation list and block geometry |
| Contiguous | `ceil(size / block_size)` data blocks from the first free run that fits |
| Linked | A 4-byte next pointer occupies part of every data block, so `ceil(size / (block_size - 4))` blocks are taken in ascending free-block order |
| Indexed | `ceil(size / block_size)` data blocks plus one index block allocated first; the index holds `block_size / 4` pointers and an over-capacity file is rejected |
| Failures | Duplicate ID, no space, no contiguous run, over-capacity, and missing file are reported with the reason; allocation is atomic, so a failure changes nothing |
| Output | Per method: every operation, `Block usage`, `Summary`, `Space: requested ... allocated ... wasted ...; metadata blocks ...`, then shared Notes |
| Waste | Allocated block bytes minus requested bytes over active files; includes last-block slack, link-pointer slots and index blocks, which are also the metadata-block count |
| Oracle fixtures | Mixed 30/94/158 wasted with 5/6/7 used blocks; no-space, reuse, contiguous fragmentation, index capacity and minimum-disk cases all match hand calculations and an independent Python model |

Reporting (M8) - one shared layer, five comparison modules:

| Module | CSV | Columns after `algorithm` | Chart |
|---|---|---|---|
| CPU scheduling | `cpu_scheduling.csv` | `avg_waiting_time`, `avg_turnaround_time`, `avg_response_time`, `quantum`, `process_count` | 3 panels, unchanged |
| Page replacement | `page_replacement.csv` | `hits`, `faults`, `frame_count`, `reference_count` | hits vs faults |
| Memory allocation | `memory_allocation.csv` | `allocated_requests`, `failed_requests`, `internal_fragmentation`, `largest_free_block`, `block_count`, `request_count` | 2x2 panels |
| Disk scheduling | `disk_scheduling.csv` | `total_head_movement`, `request_count`, `initial_head`, `min_cylinder`, `max_cylinder` | one panel |
| File allocation | `file_allocation.csv` | `used_blocks`, `free_blocks`, `active_files`, `allocations_ok`, `allocations_failed`, `deletions_ok`, `deletions_failed`, `retrievals_ok`, `retrievals_failed`, `requested_bytes`, `allocated_bytes`, `wasted_bytes`, `metadata_blocks`, `block_count`, `block_size`, `operation_count` | 2x2 panels |

All values are simulated results, never implementation runtimes. PNG names are
`<module>_comparison.png` under `data/results/`. Banker's algorithm, the POSIX
demonstration and the IPC demonstration remain console-only.

## 6. Behavior and conventions

- Metrics: `TAT = CT - AT`, `WT = TAT - BT`, `RT = first CPU start - AT`.
- Priority: the ordering is chosen at runtime (option 1: smaller number is
  higher priority; option 2: larger number is higher priority). It affects only
  the Priority algorithm.
- Ties in SJF/Priority: earlier arrival, then smaller PID.
- RR: arrivals that occur during a quantum enter the queue before the
  preempted process.
- FCFS: input order; input is stable-sorted by arrival before everything runs.
- Output paths are resolved from the project root, not the current directory.
  Root = `OS_TOOLKIT_ROOT` if set, otherwise two levels above `/proc/self/exe`.
- Environment switches: `OS_TOOLKIT_NO_OPEN=1` skips the image viewer,
  `OS_TOOLKIT_NO_PLOT=1` skips plot generation entirely (used by tests).
- Page replacement: FIFO replaces in load order; LRU uses the oldest
  `last_used`; LFU uses the smallest `uses`, then the earliest `loaded_at`.
- Memory allocation: First Fit scans from block 1; Best Fit picks the smallest
  sufficient free block; Worst Fit picks the largest free block. Ties go to the
  lowest-numbered block. A failed request consumes no space.
- Memory reporting: per-request block choice, remaining free space per block,
  allocated/failed counts, internal fragmentation (always 0) and external
  fragmentation (largest free block).
- POSIX demo: every result comes from a real Linux system call. File writes are
  confined to one `mkstemp()` file under `/tmp`; the module never modifies
  project files. Child processes use `_exit()` on failure and are always reaped.
- Simulated modules (scheduling, paging, memory allocation, deadlock avoidance,
  disk scheduling, file allocation) are labeled as simulations; POSIX and IPC
  results are real. Never present a simulated metric as measured runtime.
- Reporting (M8): CPU scheduling, page replacement, memory allocation, disk
  scheduling and file allocation write `data/results/<module>.csv` and a
  `<module>_comparison.png`. Banker's algorithm, the POSIX demo and the IPC demo
  stay console-only because they have no comparable alternatives. CSV convention:
  `algorithm` first, snake_case metrics, workload context last, two decimals, one
  row per algorithm, no timestamp or runtime column.
- Reporting failures are non-fatal: a failed CSV open/write/close prints
  `CSV export failed; the algorithm results above are still valid.` and skips the
  plot; a failed plot prints `Could not generate the graph.` and keeps the CSV.
- Style: C11, 4-space indent, `-Wall -Wextra -Wpedantic`, no tabs in source,
  comments in plain English, one module per directory under `src/`.
- The IPC module needs `-pthread` in `CFLAGS`; the same variable is used on the
  link line, so the flag is not repeated. Never remove it while threads or
  `sem_init()` remain in the build.
- Producer-consumer results are nondeterministic by design: which producer takes
  which ID and how many items each consumer handles changes per run. Only the
  totals, the duplicate/missing/invalid counters and the final ring state are
  stable, which is what the fixtures assert.
- Deadlock avoidance is deterministic: the lowest-numbered ready process is
  chosen first. An Allocation entry above the corresponding Maximum rejects the
  whole state instead of being re-prompted, because the state itself is
  inconsistent rather than the individual value being out of range.
- Disk scheduling: bounds are entered first and every later value is validated
  against them. SSTF selects by (distance, lower cylinder, earlier queue index).
  SCAN always starts upward and pays for the run to the maximum cylinder before
  reversing; the downward sweep stops at the last request. Never silently
  convert this to LOOK.
- File allocation: sizes are bytes; contiguous and indexed data blocks hold the
  full block, linked blocks reserve 4 bytes for the next pointer. Indexed files
  use exactly one index block with `block_size / 4` pointer slots and are
  rejected above that capacity. Allocation checks run before any mutation, so a
  failed request leaves the volume unchanged. Wasted space includes all block
  slack, pointer slots and index blocks; never report a real-filesystem effect.

## 7. Traps (already hit once)

- Do not build with the Windows MSYS2 GCC; `mkdir("data", 0777)` fails there.
- GCC 11 emits a false `-Wstringop-overflow` for `print_results` when it takes a
  `Process p[]` plus a `const char *name` argument. Fixed by passing an
  algorithm index and using an internal name table. Do not "fix" it by
  disabling the warning.
- `xdg-open ... &` always returns 0, so a failure check after it is dead code.
- Piping program output through `grep` inside `wsl bash -c "..."` from
  PowerShell truncates the output; redirect to a file and grep in a second step.
- PowerShell mangles heredocs containing quotes; write a temporary `.sh` file
  and run it with `bash`, then delete it.
- The program prompts one value per line; multi-value lines break the input
  sequence.
- The working tree is checked out with CRLF while the git index stores LF, so a
  plain `git diff` shows every text file as rewritten. Read real changes with
  `git diff --ignore-cr-at-eol` and do not re-normalize line endings in a
  feature commit.
- The paging summary is followed by a note line that also starts with `LFU `;
  filter numeric rows (`^LFU +[0-9]`) rather than the bare policy name.
- All input fixtures now exit with menu `10`; `9` is About after the M7 menu
  renumbering. A fixture still ending in `9` will print About and then exit on
  EOF, hiding the mistake because test output is discarded. `read_choice()`
  returns 10 on EOF for the same reason: with 9 as the About entry, an EOF value
  of 9 would loop on About forever.
- The four invalid-menu fixtures (`08_invalid_input`, `paging_05_invalid_input`,
  `memory_05_invalid_input`, `file_06_invalid_input`) feed `11` as the
  out-of-range menu value; `10` is now a valid exit. The IPC submenu is
  unchanged (1 producer-consumer, 2 pipe, 3 back), so its `3` return values were
  not renumbered; `ipc_03_invalid` still feeds `9` as an invalid submenu choice.
- A disk fixture must contain exactly `6 + requests + 1` values. A file fixture
  contains four leading values, three values per allocation, two per delete or
  retrieval, and the trailing `10`. An extra value is read as the next menu
  choice, so count the lines when editing.
- A CPU fixture contains the menu choice, the process count, the priority
  ordering, three values per process, the quantum and the exit choice. An extra
  or missing line is read as the next prompt, so count the lines when editing.
- `file_06_invalid_input` uses a marker checklist because it asserts every
  rejection message and the final clean run; the other six file fixtures
  compare the whole report block between the `FILE ALLOCATION` and `MAIN MENU`
  headers.
- `disk_06_invalid_input` uses a marker checklist because it asserts the exact
  rejection messages for a cylinder above the maximum, a non-numeric value and
  a negative value; the other five disk fixtures compare the whole report block
  between the `DISK SCHEDULING` and `MAIN MENU` headers.
- The memory test compares the report captured between the `MEMORY ALLOCATION`
  and `MAIN MENU` headers, with blank lines removed.
- POSIX output contains PIDs, a random temporary name and live `/tmp` entries;
  never compare it byte-for-byte. The fixture is a marker checklist, and the
  runner also requires all four child PID records and checks every child PPID
  against the parent PID, that the reported temporary path is gone, and that
  the working directory stayed empty.
- Fixture files created on Windows may contain CRLF. The POSIX checklist strips
  a trailing `\r`; use `tr -d '\r'` or equivalent for any new text comparison.
- Do not run the POSIX module through a shell pipeline that hides stderr; the
  failed-exec check expects `[exec] expected execl failure:` on stderr.
- ThreadSanitizer aborts with `FATAL: ThreadSanitizer: unexpected memory mapping`
  on this kernel. Run it under `setarch $(uname -m) -R` instead of changing any
  code.
- IPC output contains thread interleaving, so never compare it byte-for-byte.
  The fixtures are marker checklists, and the integrity line carries the totals
  that matter.
- A producer-consumer fixture that hangs usually means the stop/wake handling or
  the consumer reservation logic changed; run the case under `timeout` and check
  that every consumer decrements `remaining` exactly once.
- A Banker's fixture must contain exactly `3 + 2 * processes * resources +
  resources + 1` values. An extra value is silently read as the next menu
  choice, so the case can still pass while testing the wrong state; count the
  lines when adding or editing a fixture.
- Banker's needs the matrix parameter without `const`: C11 does not convert
  `int (*)[N]` to `const int (*)[N]` (C23 does), and `-Wpedantic` warns if it is
  declared const. Do not "fix" the warning by dropping `-Wpedantic`.
- The shared reporting lines (`Comparison data saved to ...` and `Plot generation
  skipped ...`) are emitted inside the report block captured by the memory, disk
  and file comparisons; those runners delete them with `sed` before diffing.
  Keep that filter when changing the capture, or every full-report fixture fails.
- Reporting and failure tests must use an isolated `OS_TOOLKIT_ROOT`. A regular
  file at `$root/data` forces the CSV path to fail; a root without
  `scripts/plot_results.py` forces plot failure. Never point these cases at the
  project root, or real generated files will be overwritten.
- The clean-tree check copies the working tree without `build/`, `data/results/`,
  `.git/`, `.venv/` and `__pycache__/`; it is not `git clone`, because the M0-M8
  work is uncommitted and the only commit is the pre-M0 baseline.
- A menu-dispatch input is built by dropping the trailing `10` from existing
  fixtures (`sed '$d'`) and stripping CR. Reuse that trick when adding a module
  instead of duplicating input files.

## 8. Resume checklist

1. Read `PLAN.md` milestone table and the current milestone checklist.
2. Run the build command; it must be warning-free.
3. Run `make test`; it must pass.
4. Continue with the first unchecked task in the current milestone.
5. After each milestone: build, test, update `PLAN.md`, this file, README, and
   `PROJECT_KNOWLEDGE_BASE.md`, then report.
6. Never commit until the whole project is implemented and the user explicitly
   authorizes it. The current instruction is no GitHub commit during M0-M8.

## 9. Open decisions

| Topic | Current decision |
|---|---|
| Warning fix | Pass an algorithm index to `print_results` (verified). |
| Path resolution | `/proc/self/exe` with `OS_TOOLKIT_ROOT` override (Linux-only project). |
| Test oracle | Generated CSV compared against fixtures, CRLF-insensitive. Paging compares numeric summary rows only. Memory and disk scheduling compare the full report block. File allocation compares the full report block for six fixtures and uses markers for the invalid-input case; an independent Python model cross-checked every event. POSIX uses required markers plus PID/temp-cleanup assertions. IPC uses required markers plus an exit-status check, because thread interleaving is nondeterministic. Deadlock avoidance uses required markers for the reported matrices, verdict and sequence, plus an exit-status check. Five reporting checks compare the new module CSVs against `reporting_*.expected.csv` and assert the save and plot-skip messages; one menu-dispatch chain runs choices 1-8, About (9), an invalid choice (11) and Exit (10); two CSV-failure and two plot-failure checks use isolated roots. |
| Plot in tests | Disabled via `OS_TOOLKIT_NO_PLOT=1` so tests do not need Matplotlib. Chart generation was verified separately with system Python 3 and Matplotlib 3.10.8 for all five modules. |
| Menu layout | Final: implemented modules occupy options 1-8 (8 is file allocation); About 9, Exit 10. No placeholder entries remain. |
| Paging initial state | Every policy starts with empty frames (Phase B). |
| LFU tie-break | Smallest access count, then earliest loaded page. |
| Module reporting | CSV and chart for the five algorithm-comparison modules through the shared layer (M8); Banker's, POSIX and IPC stay console-only. |
| Memory allocation model | Exact split (allocated = requested), so internal fragmentation is 0; external fragmentation = largest free block. |
| Shared memory (Q1) | Excluded (2026-10-22). Phase E uses pipes and pthreads + POSIX semaphores; no shared-memory code. |
| POSIX demo model | Real system calls only; one `mkstemp()` file; no pipes/shared memory/IPC; console only (no CSV or chart by design). |
| IPC demo model | Real pthreads, POSIX unnamed semaphores and one `pipe()` between parent and child; console only (no CSV or chart by design). |
| Deadlock model | Static-state safety analysis only; lowest-index-first sequence; an unsafe verdict never claims a deadlock; Allocation above Maximum rejects the state. |
| Disk scheduling model | Shared queue/head/bounds; SSTF tie-break (distance, lower cylinder, earlier index); conventional SCAN with fixed increasing direction and counted boundary travel; CSV and chart. |
| Disk test oracle | Five fixtures compare the whole report block; `disk_06_invalid_input` uses markers for the rejection messages. An independent Python model of the three policies also matched the binary on 300 random states. |
| File allocation model | Byte-sized files on 1-128 blocks of 8-4096 bytes; first-fit; linked blocks reserve a 4-byte pointer; one index block per indexed file with `block_size / 4` slots; atomic allocation; waste includes slack, pointer slots and index blocks. |
| File allocation oracle | Six fixtures compare the whole report block; `file_06_invalid_input` uses markers for the rejection messages. An independent Python model matched every event, mapping, used/free set and waste total. |
| Reporting scope | Five algorithm-comparison modules only; no artificial metrics for Banker's, POSIX or IPC. |
| CSV convention | `algorithm` first, snake_case metrics, workload context last, two-decimal values, one row per algorithm, no timestamp/runtime column. |
| Runtime timing | No wall-clock or `CLOCK_MONOTONIC` code; simulated metrics are never described as execution time. B5 remains out of scope. |
| Clean-tree check | Copy of the working tree without `build/`, `data/results/`, `.git/`, `.venv/` and `__pycache__/`, because the M0-M8 changes are uncommitted. |
| Commit policy | No commit until the complete project is implemented and explicitly authorized. |
