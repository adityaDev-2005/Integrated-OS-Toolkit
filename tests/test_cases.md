# Test Cases - CPU Scheduling, Page Replacement, Memory Allocation, POSIX, IPC, Deadlock Avoidance, Disk Scheduling, File Allocation and Reporting

Run the suite with:

```bash
make test
```

`tests/run_tests.sh` executes every `tests/cases/*.in` file, runs the binary
from a temporary working directory, and compares the result with the matching
fixture. Carriage returns are ignored so the fixtures survive a Windows
checkout. Every input fixture ends with the exit choice `10`; fixtures that
test an invalid menu choice feed `11`.

Fixtures whose name starts with `paging_` compare the reported hit and fault
counts with a `*.expected.txt` file. Fixtures whose name starts with `memory_`
compare the full memory-allocation report with a `*.expected.txt` file.
Fixtures whose name starts with `posix_` use a marker checklist plus PID and
cleanup assertions, because PIDs, temporary names and directory entries are
dynamic. Fixtures whose name starts with `ipc_` also use a marker checklist,
because the scheduler decides how work splits between producer and consumer
threads, so only the summary lines are stable. Fixtures whose name starts with
`banker_` use the same marker checklist and compare the reported matrices, the
safety verdict and the safe sequence or unfinished processes. Fixtures whose
name starts with `disk_` compare the full disk-scheduling report (head path and
total head movement for FCFS, SSTF and SCAN) with a `*.expected.txt` file;
`disk_06_invalid_input` uses the marker checklist because it asserts the exact
rejection messages. Fixtures whose name starts with `file_` compare the full
file-allocation report for all three methods with a `*.expected.txt` file;
`file_06_invalid_input` uses the marker checklist because it asserts every
rejection message. Every other fixture compares
`data/results/cpu_scheduling.csv` with a `*.expected.csv` file.

The five comparison modules also write a CSV through the shared reporting
layer. Five reporting fixtures (`reporting_*.expected.csv`) pin those schemas,
and the integration checks at the end of this file cover the menu dispatch, a
blocked CSV destination and an unusable plotting script. No fixture depends on
wall-clock time, because the project reports simulated algorithm metrics only.

Plot generation and image opening are disabled during the run
(`OS_TOOLKIT_NO_PLOT=1`, `OS_TOOLKIT_NO_OPEN=1`), so the suite needs neither
Matplotlib nor a desktop session. One extra check confirms that
`OS_TOOLKIT_ROOT` redirects the generated files. The memory, disk and file
runners drop the two per-module reporting lines before comparing a full
report with its fixture, so those algorithm checks stay independent of the
reporting layer. The suite currently has 52 checks: eight scheduling, five
page-replacement, five memory-allocation, one POSIX, four IPC, five
deadlock-avoidance, six disk-scheduling, seven file-allocation, one
root-override and ten reporting/integration checks.

## CPU scheduling cases

Input files contain one value per line: menu choice `1`, process count,
priority ordering (`1` = smaller numbers first, `2` = larger numbers first),
then `arrival`, `burst`, `priority` for each process, then the Round Robin
quantum, then the exit choice `10`. All eight fixtures select ordering `1`, so
the expected results below are unchanged.

| Fixture | Workload | Quantum | Purpose |
|---|---|---|---|
| `01_documented` | P1(0,5,2) P2(1,3,1) P3(2,8,4) P4(3,6,3) | 3 | Sample workload from the project knowledge base |
| `02_single_process` | P1(5,4,1) | 2 | Single process that arrives after time 0 |
| `03_idle_gap` | P1(0,2,1) P2(5,3,2) | 2 | CPU idle between two processes |
| `04_ties` | P1(0,5,2) P2(0,5,2) | 2 | Equal arrival, burst, and priority |
| `05_quantum_covers_burst` | P1(0,3,1) P2(1,2,2) | 10 | Quantum larger than every burst |
| `06_quantum_one` | P1(0,3,1) P2(1,2,2) | 1 | Maximum preemption |
| `07_arrival_during_quantum` | P1(0,4,1) P2(1,1,2) | 2 | Arrival during a running quantum |
| `08_invalid_input` | same as `02` after rejected values | 2 | Invalid menu, count, arrival, and quantum |

`08_invalid_input` feeds `11`, `abc`, `-5`, and `0` where a valid value is
required. Every invalid value must be rejected and re-prompted; the run must
end with the same results as `02_single_process`.

### Expected results

Average waiting time / average turnaround time / average response time.

| Fixture | FCFS | SJF | Priority | Round Robin |
|---|---|---|---|---|
| `01_documented` | 5.75 / 11.25 / 5.75 | 5.25 / 10.75 / 5.25 | 5.25 / 10.75 / 5.25 | 8.50 / 14.00 / 3.00 |
| `02_single_process` | 0.00 / 4.00 / 0.00 | 0.00 / 4.00 / 0.00 | 0.00 / 4.00 / 0.00 | 0.00 / 4.00 / 0.00 |
| `03_idle_gap` | 0.00 / 2.50 / 0.00 | 0.00 / 2.50 / 0.00 | 0.00 / 2.50 / 0.00 | 0.00 / 2.50 / 0.00 |
| `04_ties` | 2.50 / 7.50 / 2.50 | 2.50 / 7.50 / 2.50 | 2.50 / 7.50 / 2.50 | 4.50 / 9.50 / 1.00 |
| `05_quantum_covers_burst` | 1.00 / 3.50 / 1.00 | 1.00 / 3.50 / 1.00 | 1.00 / 3.50 / 1.00 | 1.00 / 3.50 / 1.00 |
| `06_quantum_one` | 1.00 / 3.50 / 1.00 | 1.00 / 3.50 / 1.00 | 1.00 / 3.50 / 1.00 | 1.50 / 4.00 / 0.00 |
| `07_arrival_during_quantum` | 1.50 / 4.00 / 1.50 | 1.50 / 4.00 / 1.50 | 1.50 / 4.00 / 1.50 | 1.00 / 3.50 / 0.50 |
| `08_invalid_input` | 0.00 / 4.00 / 0.00 | 0.00 / 4.00 / 0.00 | 0.00 / 4.00 / 0.00 | 0.00 / 4.00 / 0.00 |

The full expected CSV, including the `quantum` and `process_count` columns, is
stored next to each input file.

## Page replacement cases

Input files contain one value per line: menu choice `2`, frame count, reference
count, one page number per reference, then the exit choice `10`.

| Fixture | Frames | References | Purpose |
|---|---|---|---|
| `paging_01_classic` | 3 | 1 2 3 4 1 2 5 1 2 3 4 5 | Classic reference string where FIFO and LRU differ |
| `paging_02_single_frame` | 1 | 1 2 1 3 | Every access faults |
| `paging_03_all_hits` | 3 | 1 2 1 2 1 | No faults after warm-up |
| `paging_04_lfu_divergence` | 2 | 1 1 2 3 3 1 | LFU keeps the frequently used page; FIFO and LRU do not |
| `paging_05_invalid_input` | 3 | 1 2 3 1 2 4 after rejected values | Invalid menu, frame count, reference count, and page number |

`paging_05_invalid_input` feeds `11`, `abc`, `0`, `-1`, and `x` where a valid
value is required. Every invalid value must be rejected and re-prompted.

### Expected results

Hits / faults.

| Fixture | FIFO | LRU | LFU |
|---|---|---|---|
| `paging_01_classic` | 3 / 9 | 2 / 10 | 2 / 10 |
| `paging_02_single_frame` | 0 / 4 | 0 / 4 | 0 / 4 |
| `paging_03_all_hits` | 3 / 2 | 3 / 2 | 3 / 2 |
| `paging_04_lfu_divergence` | 2 / 4 | 2 / 4 | 3 / 3 |
| `paging_05_invalid_input` | 2 / 4 | 2 / 4 | 2 / 4 |

The exact summary lines are stored next to each input file. LFU counts accesses
while a page is resident and evicts the smallest count, breaking ties by the
earliest loaded page.

## Memory allocation cases

Input files contain one value per line: menu choice `3`, block count, one size
per block, request count, one size per request, then the exit choice `10`.

A request is granted exactly its requested size from the chosen block; the
remainder stays free and can serve later requests. Internal fragmentation is
allocated size minus requested size, which is always zero under this exact
splitting model. External fragmentation is reported as the largest free block.

| Fixture | Blocks | Requests | Purpose |
|---|---|---|---|
| `memory_01_classic` | 100 500 200 300 600 | 212 417 112 426 | Policies choose different blocks; Best Fit succeeds where the others fail |
| `memory_02_all_fail` | 50 80 | 100 200 | Every request fails; no block changes |
| `memory_03_exact_fit` | 100 200 | 200 100 | Every request exactly consumes a block; free space reaches zero |
| `memory_04_split_remainder` | 1000 | 300 300 300 | Repeated splitting leaves exactly 100 free |
| `memory_05_invalid_input` | 100 200 after rejected values | 200 100 after rejected values | Invalid menu, counts, block size and request size |

`memory_05_invalid_input` feeds `11`, `0`, `0`, `abc`, and `-5` where a valid
value is required. Every invalid value must be rejected and re-prompted; the
final report must match `memory_03_exact_fit`.

### Expected results

Allocated requests / failed requests / largest free block. Internal
fragmentation is 0 in every row.

| Fixture | First Fit | Best Fit | Worst Fit |
|---|---|---|---|
| `memory_01_classic` | 3 / 1 / 300 | 4 / 0 / 174 | 3 / 1 / 300 |
| `memory_02_all_fail` | 0 / 2 / 80 | 0 / 2 / 80 | 0 / 2 / 80 |
| `memory_03_exact_fit` | 2 / 0 / 0 | 2 / 0 / 0 | 2 / 0 / 0 |
| `memory_04_split_remainder` | 3 / 0 / 100 | 3 / 0 / 100 | 3 / 0 / 100 |
| `memory_05_invalid_input` | 2 / 0 / 0 | 2 / 0 / 0 | 2 / 0 / 0 |

The exact report lines, including per-request block choices, are stored next to
each input file.

## POSIX demonstration case

The input is menu choice `4`, then exit choice `10`. The demonstration needs no
other input. It performs real system calls, so the test does not compare the
entire output. Instead, the expected file lists required markers and the runner
adds these assertions:

- Every reported child PPID equals the parent PID printed by the module.
- The temporary path reported by `mkstemp()` no longer exists after the run.
- The temporary working directory is empty, proving no project file was written.
- The binary exits with status 0.

| Fixture | Purpose | Required evidence |
|---|---|---|
| `posix_01_demo` | Exercise process, exec, file and directory APIs | Parent/child PIDs, `waitpid()` status 42, `wait()` status 7, successful `execlp`, failed `execl` with status 127, `hello world` read-back, 11-byte `stat`, `fcntl` `O_APPEND`, unlink message, `opendir`/`readdir`/`closedir` |

The exact marker list is in `posix_01_demo.expected.txt`. All file writes are
confined to the temporary file; no pipe, shared memory or other IPC is exercised.

## IPC and synchronization cases

Input files contain one value per line: menu choice `5`, then the IPC submenu
choice (`1` producer-consumer, `2` pipe), then the demonstration inputs, then
`3` to return and the exit choice `10`.

Producer-consumer fixtures enter the buffer size, item count and both thread
counts. The demonstration keeps no file output, so the fixtures list the stable
`[pc]` markers instead of a full transcript: the configuration line, the initial
semaphore values, the integrity summary, the cleanup line and the verdict. The
integrity line is the real assertion, because it reports the produced and
consumed totals, duplicate identifiers, missing identifiers, invalid identifiers
and the final ring state.

Pipe fixtures list the `[pipe]` markers for pipe creation, closure of the unused
end in each process, the byte counts, the EOF observation, the received data,
the `waitpid()` result and the cleanup line. Process IDs never appear in a
fixture, because they change on every run.

| Fixture | Configuration | Purpose |
|---|---|---|
| `ipc_01_producer_consumer` | buffer 5, 20 items, 2 producers, 2 consumers | Normal run; every item produced and consumed exactly once, ring empty afterwards |
| `ipc_02_pipe` | pipe demonstration | Unused ends closed, 25 bytes transferred, EOF observed, data matches, child reaped |
| `ipc_03_invalid` | rejected values before a valid run | Submenu, buffer, item count and thread counts reject invalid input and re-prompt; the final run matches a normal run |
| `ipc_04_more_threads_than_items` | buffer 1, 1 item, 8 producers, 8 consumers | More threads than work; all threads terminate, peak occupancy stays at 1 of 1 slot |

`ipc_03_invalid` feeds `9`, `abc`, `0`, `101`, `20000` and `9` where a valid
value is required. The expected file requires one "Invalid input" message for
the submenu range, the buffer range, the item range and the thread range, which
proves every validation path rejects bad input.

The exact marker lists are in the matching `*.expected.txt` files. The runner
also fails the case when the binary exits with a non-zero status. Both
producer-consumer and pipe results are real runtime values; nothing in these
cases is simulated.

## Deadlock avoidance cases

Input files contain one value per line: menu choice `6`, process count, resource
type count, the Allocation matrix in row-major order, the Maximum matrix in
row-major order, the Available vector, then the exit choice `10`. Each fixture
holds exactly `3 + 2 * processes * resources + resources + 1` values.

The module prints all matrices, the derived Need matrix and the safety trace, so
the fixtures list stable markers instead of a whole transcript: a Need row, the
Available vector, the first and last safety step, the verdict, and the safe
sequence or the unfinished processes.

| Fixture | State | Purpose |
|---|---|---|
| `banker_01_safe_state` | 5 processes, 3 resources, Available (3 3 2) | Classic textbook state; reports `P2 -> P4 -> P1 -> P3 -> P5` |
| `banker_02_unsafe_state` | 3 processes, 2 resources, Available (0 0) | P1 and P2 finish, P3 cannot; the verdict is UNSAFE, never "deadlocked" |
| `banker_03_multiple_sequences` | 4 processes, 2 resources, Available (1 1) | Several valid orders exist; the lowest-index-first choice is asserted |
| `banker_04_invalid_allocation` | Allocation P1 R1 = 3, Maximum P1 R1 = 2 | The inconsistent state is rejected before any safety analysis |
| `banker_05_zero_state` | 2 processes, 2 resources, all values 0 | Zero need with zero work still finishes in order |

`banker_03_multiple_sequences` has more than one correct answer: marking P1 or
P2 first both lead to a complete sequence, so the fixture asserts the documented
rule that the lowest-numbered ready process is chosen first.

`banker_04_invalid_allocation` is the only case that never reaches the safety
algorithm; it asserts the rejection message, the `REJECTED` verdict and the
explanation line.

The 1-20 dimension limits and the 999 value limit are not fixtures, because a
20 x 20 state needs more than 800 input lines. Both limits were exercised with
an AddressSanitizer build, including 20 processes x 20 resources and a 1 x 1
state with value 999.

## Disk scheduling cases

Input files contain one value per line: menu choice `7`, minimum cylinder,
maximum cylinder, initial head position, request count, one cylinder number per
request, then the exit choice `10`. Each fixture holds exactly
`6 + requests + 1` values.

FCFS, SSTF and SCAN read the same queue and the same initial head. Every fixture
compares the full report block: the banner, the configured range, head and
request lines, the three head paths and movement totals, the SCAN direction
line, the comparison table and the notes. A head path starts at the initial
head, so the first element is not a served request.

| Fixture | Bounds | Head | Requests | Purpose |
|---|---|---|---|---|
| `disk_01_classic_both_sides` | 0-199 | 53 | 98 183 37 122 14 124 65 67 | Requests on both sides of the head; all three policies diverge |
| `disk_02_one_side` | 0-99 | 10 | 20 30 40 50 | Every request above the head; SCAN still pays for the run to 99 |
| `disk_03_boundaries` | 0-99 | 50 | 0 99 | Requests exactly on both cylinder bounds |
| `disk_04_duplicates_and_head` | 0-40 | 20 | 20 20 30 10 | Duplicate requests and two requests at the initial head |
| `disk_05_sstf_tie` | 0-99 | 50 | 80 20 | Equal SSTF distance; the lower cylinder must be served first |
| `disk_06_invalid_input` | 0-99 | 50 | 10 90 after rejected values | Rejected maximum, non-numeric value, negative head and out-of-range requests |

`disk_06_invalid_input` feeds `0` as the minimum cylinder, then `0` and `abc`
as the maximum (both rejected), `-5` and `150` as the head (both rejected), `0`
as the request count, and `200` and `-1` as request values. The expected file
requires one rejection message for each affected range (1-9999, 0-99 and
1-200), and the final report covers bounds 0-99, head 50 and requests 10 and
90, proving every rejected value was re-prompted.

### Expected results

Service order / total head movement in cylinders.

| Fixture | FCFS | SSTF | SCAN |
|---|---|---|---|
| `disk_01_classic_both_sides` | 53 -> 98 -> 183 -> 37 -> 122 -> 14 -> 124 -> 65 -> 67 / 640 | 53 -> 65 -> 67 -> 37 -> 14 -> 98 -> 122 -> 124 -> 183 / 236 | 53 -> 65 -> 67 -> 98 -> 122 -> 124 -> 183 -> 199 -> 37 -> 14 / 331 |
| `disk_02_one_side` | 10 -> 20 -> 30 -> 40 -> 50 / 40 | 10 -> 20 -> 30 -> 40 -> 50 / 40 | 10 -> 20 -> 30 -> 40 -> 50 -> 99 / 89 |
| `disk_03_boundaries` | 50 -> 0 -> 99 / 149 | 50 -> 99 -> 0 / 148 | 50 -> 99 -> 0 / 148 |
| `disk_04_duplicates_and_head` | 20 -> 20 -> 20 -> 30 -> 10 / 30 | 20 -> 20 -> 20 -> 10 -> 30 / 30 | 20 -> 20 -> 20 -> 30 -> 40 -> 10 / 50 |
| `disk_05_sstf_tie` | 50 -> 80 -> 20 / 90 | 50 -> 20 -> 80 / 90 | 50 -> 80 -> 99 -> 20 / 128 |
| `disk_06_invalid_input` | 50 -> 10 -> 90 / 120 | 50 -> 10 -> 90 / 120 | 50 -> 90 -> 99 -> 10 / 138 |

SSTF breaks ties by the lower cylinder and then by the earlier queue entry.
`disk_04_duplicates_and_head` pins the cylinder tie (three requests at 20) and
the zero-distance case; `disk_05_sstf_tie` pins the equal-distance rule through
the reported path (50 -> 20 -> 80), because both orders total 90 cylinders. SCAN
always starts upward and pays for the run to the maximum cylinder even when no
request lies there: `disk_02_one_side` reaches 99 for 89 cylinders, and
`disk_01` reaches 199 between 183 and 37. A LOOK implementation would report 40
and 299 for those two rows instead, so the fixtures distinguish the policies.
All three totals were cross-checked against an independent Python model of the
policies over 300 seeded random states and five edge states before the fixtures
were finalized.

## File allocation cases

Input files contain one value per line: menu choice `8`, block count, block
size in bytes, operation count, then per operation the type (`1` allocate,
`2` delete, `3` retrieve) and the file ID, plus the file size in bytes for an
allocation, then the exit choice `10`. A fixture holds four leading values plus
three values per allocation and two per delete or retrieval, plus the trailing
exit value.

Each method runs on an independent copy of the same empty disk and receives the
same operations, so every fixture compares the full report block for all three
methods: every operation line, `Block usage` with used and free block lists,
the `Summary` counters, and `Space: requested ... allocated ... wasted ...;
metadata blocks ...`. The report also states that no real filesystem is read or
modified. Wasted space is allocated block bytes minus requested bytes over the
active files, so it includes last-block slack, link-pointer slots and index
blocks.

| Fixture | Disk | Operations | Purpose |
|---|---|---|---|
| `file_01_mixed` | 10 x 64 | 10 | Allocate, retrieve, delete and re-allocate in every method; mappings and waste diverge |
| `file_02_no_space` | 4 x 64 | 4 | The 250-byte file needs 5 linked/indexed blocks but only 4 exist; contiguous fits it exactly |
| `file_03_reuse` | 6 x 64 | 10 | Deleted blocks are reused by later files; unaffected files keep their mapping |
| `file_04_contig_fragmentation` | 10 x 100 | 15 | Three free blocks exist but no run of two, so contiguous allocation fails while linked and indexed succeed |
| `file_05_index_capacity` | 8 x 8 | 4 | The index holds 2 pointers; a 24-byte file needs 3 data blocks and is rejected |
| `file_06_invalid_input` | 4 x 64 | 3 | Invalid menu, block count, block size, operation count, type, file ID and size are all rejected and re-prompted |
| `file_07_min_disk` | 1 x 8 | 7 | Minimum disk: contiguous and linked fit a 4-byte file, indexed needs an index plus data and cannot |

### Expected results

Used blocks of the disk / wasted bytes over active files / metadata blocks.

| Fixture | Contiguous | Linked | Indexed |
|---|---|---|---|
| `file_01_mixed` | 5 of 10 / 30 / 0 | 6 of 10 / 94 / 0 | 7 of 10 / 158 / 2 |
| `file_02_no_space` | 4 of 4 / 6 / 0 | 2 of 4 / 64 / 0 | 2 of 4 / 64 / 1 |
| `file_03_reuse` | 2 of 6 / 0 / 0 | 4 of 6 / 128 / 0 | 4 of 6 / 128 / 2 |
| `file_04_contig_fragmentation` | 7 of 10 / 0 / 0 | 7 of 10 / 300 / 0 | 7 of 10 / 300 / 3 |
| `file_05_index_capacity` | 4 of 8 / 0 / 0 | 8 of 8 / 32 / 0 | 2 of 8 / 8 / 1 |
| `file_06_invalid_input` | 0 of 4 / 0 / 0 | 0 of 4 / 0 / 0 | 0 of 4 / 0 / 0 |
| `file_07_min_disk` | 1 of 1 / 4 / 0 | 1 of 1 / 4 / 0 | 0 of 1 / 0 / 0 |

The exact report lines, including every block mapping, are stored next to each
input file. Key mappings:

- `file_01_mixed`: file 4 ends in blocks [2 3 4] (contiguous), [2 3 4 5]
  (linked) and index 3 / data [4 5 6] (indexed).
- `file_02_no_space`: contiguous stores file 1 in [0 1 2 3] and rejects file 2;
  linked and indexed reject file 1 with `needs 5, 4 free` and store file 2.
- `file_03_reuse`: file 3 reuses file 1's freed blocks [0] (contiguous) or
  [0 1] (linked and indexed), and file 4 later reuses file 2's blocks.
- `file_04_contig_fragmentation`: the 200-byte request fails with
  `no contiguous run of 2 blocks (3 free)`; linked stores it in [0 1 4] and
  indexed in index 0 / data [1 4]. Contiguous waste is 0 because only active
  files count; linked and indexed reports waste 300 bytes for the 400 requested.
- `file_05_index_capacity`: indexed rejects the 24-byte file with
  `needs 3 data blocks, one index block holds 2`.
- `file_07_min_disk`: indexed rejects the 4-byte file with `needs 2, 1 free`,
  then every retrieval and deletion of the missing file is reported as failed.

`file_06_invalid_input` feeds `11` as the menu choice, then `0`, `129` and
`abc` as the block count, `7` and `4097` as the block size, `0` and `65` as the
operation count, `4` and `0` as the operation type, `0` and `33` as the file ID,
and `0` and `1000001` as the file size. The expected file requires one rejection
message for each affected range (1-128, 8-4096, 1-64, 1-3, 1-32 and
1-1000000), plus the final mappings for the clean 3-operation run and an empty
final disk.

## Reporting output

The shared writer (`src/benchmark/csv_logger.c`) produces one CSV per
comparison module under `data/results/` and keeps the conventions of the
original CPU scheduling export:

- `algorithm` is always the first column, with one row per algorithm.
- Metric columns use `snake_case` names and come first; the workload-context
  columns that explain how the row was produced come last.
- Real-valued metrics use two decimals (`%.2f`); counts are integers.
- There is no timestamp column and no wall-clock or runtime column.

| Module | CSV | Columns after `algorithm` |
|---|---|---|
| CPU scheduling | `cpu_scheduling.csv` | `avg_waiting_time`, `avg_turnaround_time`, `avg_response_time`, `quantum`, `process_count` |
| Page replacement | `page_replacement.csv` | `hits`, `faults`, `frame_count`, `reference_count` |
| Memory allocation | `memory_allocation.csv` | `allocated_requests`, `failed_requests`, `internal_fragmentation`, `largest_free_block`, `block_count`, `request_count` |
| Disk scheduling | `disk_scheduling.csv` | `total_head_movement`, `request_count`, `initial_head`, `min_cylinder`, `max_cylinder` |
| File allocation | `file_allocation.csv` | `used_blocks`, `free_blocks`, `active_files`, `allocations_ok`, `allocations_failed`, `deletions_ok`, `deletions_failed`, `retrievals_ok`, `retrievals_failed`, `requested_bytes`, `allocated_bytes`, `wasted_bytes`, `metadata_blocks`, `block_count`, `block_size`, `operation_count` |

The console-only modules (Banker's algorithm, the POSIX demonstration and the
IPC demonstration) are not forced into this schema: they do not compare
algorithms over a shared metric, so summarizing them into rows would invent
values the project never computed. Each comparison module prints
`Comparison data saved to data/results/<module>.csv` after a successful write,
then prints `Plot generation skipped (OS_TOOLKIT_NO_PLOT is set).` when plot
generation is disabled.

`benchmark_generate_plot()` runs `scripts/plot_results.py` with the module
name, preferring `.venv/bin/python` and falling back to `python3`. The script
accepts `cpu_scheduling`, `page_replacement`, `memory_allocation`,
`disk_scheduling` and `file_allocation`, derives the input CSV and output PNG
from the project root, and exits with status 1 for an unknown module, a
missing or unreadable CSV, missing columns, an empty CSV, an invalid value or
an unwritable PNG. `OS_TOOLKIT_NO_PLOT` skips generation; `OS_TOOLKIT_NO_OPEN`
suppresses the image viewer. If no interpreter can run the script, or the PNG
is not produced, the module prints `Could not generate the graph.` and states
that the CSV and the algorithm results are still valid.

## Reporting and integration checks

Ten checks follow the fixture loop. All of them run from a temporary working
directory and require a zero exit status.

The five reporting checks rerun one existing workload per comparison module
under an isolated `OS_TOOLKIT_ROOT`:

| Label | Workload | Compared CSV |
|---|---|---|
| `reporting-cpu` | `01_documented` | `cpu_scheduling.csv` |
| `reporting-paging` | `paging_01_classic` | `page_replacement.csv` |
| `reporting-memory` | `memory_01_classic` | `memory_allocation.csv` |
| `reporting-disk` | `disk_01_classic_both_sides` | `disk_scheduling.csv` |
| `reporting-file` | `file_01_mixed` | `file_allocation.csv` |

Each reporting check compares the generated CSV with its
`reporting_*.expected.csv` fixture (the CPU module reuses its original
fixture), requires the save and plot-skip messages, and fails when a `data`
directory appears in the current working directory. Reusing existing
workloads is deliberate: the reporting layer must not change algorithm
results.

`menu-dispatch` builds one input from the scheduling, paging, memory, POSIX,
IPC, Banker's, disk and file fixtures. It drops each trailing exit choice,
concatenates the rest and appends About (`9`), an invalid choice (`11`) and
Exit (`10`), so one process visits every module in menu order. The check
requires every module banner, the About heading, the `Invalid choice. Select 1
to 10.` message, the exit message and exactly five `Comparison data saved to
data/results/` lines. This proves the complete menu and dispatch path,
including the modules that intentionally write no CSV.

The two CSV-failure checks (`csv-failure-cpu`, `csv-failure-paging`) create a
regular file where the `data` directory must go, so every CSV operation
fails. Each run must still exit 0, keep the algorithm results on screen,
print `CSV export failed; the algorithm results above are still valid.`,
report no saved CSV and skip plot generation entirely (no `Generating `
line), because a failed export leaves nothing to plot.

The two plot-failure checks (`plot-failure-cpu`, `plot-failure-paging`) use an
isolated root without `scripts/plot_results.py`. The CSV must still exist and
be non-empty, the save message must appear, and the module must explain the
failure with `Could not generate the graph.` and `The CSV file and the
algorithm results are still valid.` A stale `Graph saved to ...` message
fails the check.

## Manual checks

These need a desktop session, so they stay manual:

1. Run a module with a desktop available and confirm that the generated chart
   opens (or that the program reports that it could not open it).
2. Run with `OS_TOOLKIT_NO_PLOT=1` and confirm that no PNG is written.

Chart generation was also verified outside the suite with the system `python3`
and Matplotlib 3.10.8: all five modules produced their PNG. The automatic
plot-failure checks force the same command-failure branch by using an
isolated root without `scripts/plot_results.py`; a missing Matplotlib
installation is another way to reach that branch.
