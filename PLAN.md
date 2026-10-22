# Integrated OS Toolkit - Implementation Plan and Progress

> **Status:** M0-M8 complete (stabilization, page replacement, memory allocation, POSIX demonstration, IPC and synchronization, deadlock avoidance, disk scheduling, file allocation, benchmarking and final integration); Q1 resolved (shared memory excluded); Phase I reporting generalized and verified
> **Last updated:** 2026-10-23
> **Companion documents:** `AGENT_CONTEXT.md` (continuity notes for resuming work),
> `PROJECT_KNOWLEDGE_BASE.md` (project reference maintained by the team)

## How this file is used

- `[ ]` = not done, `[x]` = done and verified.
- Every milestone ends with: warning-free build, regression run, README and
  knowledge-base updates, and a short report.
- Update the milestone table, the checklists, and the progress log in the same
  change that completes the work.
- Do not mark an item complete until the build and the relevant test confirm it.

## Scope guardrails

- `PROJECT_KNOWLEDGE_BASE.md` is the authoritative specification. This file is
  the execution roadmap and must not widen that scope.
- Only the modules, algorithms, demonstrations, metrics and infrastructure
  named by the knowledge base are implemented.
- The B1-B8 backlog is out of scope and is not scheduled.
- All five algorithm-comparison modules (CPU scheduling, page replacement,
  memory allocation, disk scheduling and file allocation) report through the
  shared benchmarking layer added in M8. Banker's algorithm, the POSIX
  demonstration and the IPC demonstration stay console-only because they have no
  comparable algorithm alternatives; no artificial metrics are invented for
  them.
- Shared memory is excluded from the project scope (Q1, resolved 2026-10-22).
  Phase E uses pipes, pthreads and POSIX semaphores only; no shared-memory code
  is written.
- Simulated OS metrics stay separate from measured implementation runtime.
- The POSIX demonstration uses real system calls only. Never replace actual
  PIDs, wait statuses or file results with fabricated output; keep simulated
  algorithm modules clearly labeled as simulations.

## Verified baseline (before M0)

| Check | Result |
|---|---|
| Repository | `D:\OS_project\Integrated-OS-Toolkit`, branch `main`, commit `db7a6c4` |
| Build | Succeeds in WSL `NVIDIA-Workbench` (GCC 11.4), but 4 `-Wstringop-overflow` warnings at `src/scheduling/scheduling.c:450-454` |
| Sample workload | Matches the Review 1 table exactly: FCFS 5.75/11.25/5.75, SJF 5.25/10.75/5.25, Priority 5.25/10.75/5.25, RR 8.50/14.00/3.00 |
| Implemented code | `src/main.c`, `src/scheduling/scheduling.c`, `scripts/plot_results.py` |
| Placeholders | 9 tracked files are 0 bytes: `common.h`, `benchmark.h`, `paging.h`, `paging.c`, `benchmark.c`, `csv_logger.c`, `process_demo.c`, `producer_consumer.c`, `test_cases.md` |
| Generated artifacts | `data/` is untracked and not ignored |

## Milestone overview

| ID | Milestone | KB phase | Status | Acceptance summary |
|---|---|---|---|---|
| M0 | Stabilization | Phase A | Done | 0 warnings, path-independent output, regression tests, docs synced |
| M1 | Page replacement (FIFO/LRU/LFU) | Phase B | Done | 3 policies on shared input, independent empty frames, hits and faults compared, menu wired |
| M2 | Memory allocation (First/Best/Worst Fit) | Phase C | Done | Three policies on shared input, exact splitting, fragmentation and failures reported, menu wired |
| M3 | POSIX process demo | Phase D | Done | Real fork/exec/wait/IDs, open/read/write/close/lseek/stat/fcntl, directory listing, error and cleanup paths verified |
| M4 | IPC and synchronization | Phase E | Done | pthreads + semaphores bounded buffer with integrity check, fork/pipe demo with EOF and child reaping, menu wired |
| M5 | Deadlock avoidance (Banker's) | Phase F | Done | Validated matrices, rejected inconsistent states, safe sequence or unsafe verdict, menu wired |
| M6 | Disk scheduling (FCFS/SSTF/SCAN) | Phase G | Done | Shared queue and head, documented tie-break and SCAN direction, head movement compared, menu wired |
| M7 | File allocation (contiguous/linked/indexed) | Phase H | Done | Simulated block model with documented metadata, allocation/deletion/retrieval, block usage and waste reporting, menu wired |
| M8 | Benchmarking and final integration | Phase I | Done | Shared CSV/plot layer in five comparison modules, non-fatal CSV/plot failures, generalized charts, clean-tree build and 52 checks pass |

## M0 - Stabilization (Phase A)

### Scope

Finish the remaining Phase A items and remove defects found during verification.
Algorithm behavior and printed metrics must not change.

### Tasks

- [x] M0.1 Remove the 4 GCC 11 `-Wstringop-overflow` warnings by making
      `print_results` take an algorithm index and use an internal name table
      (verified approach: 0 warnings, identical output).
- [x] M0.2 Add `include/common.h` and `src/common/common.c` with project-root
      resolution: `OS_TOOLKIT_ROOT` override, otherwise derived from
      `/proc/self/exe` (binary lives in `<root>/build/`).
- [x] M0.3 Make CSV output path-independent; create `data/results` under the
      project root regardless of the current working directory.
- [x] M0.4 Make plot invocation robust: absolute script path, prefer
      `.venv/bin/python`, fall back to `python3`, check the exit status, report
      failures clearly, and support `OS_TOOLKIT_NO_OPEN` and
      `OS_TOOLKIT_NO_PLOT`.
- [x] M0.5 Skip `xdg-open` when no `DISPLAY`/`WAYLAND_DISPLAY` is present; stop
      backgrounding the command so failures are observable.
- [x] M0.6 Check CSV write and close results; report I/O errors.
- [x] M0.7 Remove dead code: unreachable `return count;` and the commented-out
      `save_csv` call.
- [x] M0.8 Add `data/results/` to `.gitignore`.
- [x] M0.9 Fill `tests/test_cases.md` with the regression matrix and expected
      values; add `tests/run_tests.sh` plus `tests/cases/*.in` and
      `*.expected.csv` fixtures.
- [x] M0.10 Add `make test` and include `src/common/common.c` in the build.
- [x] M0.11 Update `README.md` (status table, correct checkboxes, remove unused
      pandas mention, document path independence and tests).
- [x] M0.12 Sync `PROJECT_KNOWLEDGE_BASE.md` (status table, repository tree,
      limitations, Phase A checkboxes).

### Acceptance

- `make clean && make` produces no warnings.
- The application runs correctly from any working directory.
- `make test` passes every fixture.
- README and knowledge base describe the actual behavior.

### Result (verified 2026-10-22)

| Check | Result |
|---|---|
| `make clean && make` | 0 warnings, 0 errors |
| `make test` | 9/9 checks pass (8 fixtures + root override) |
| Run from `/mnt/d/OS_project` (outside the repo) | CSV and PNG written under the project `data/results/`; nothing written to the working directory |
| `OS_TOOLKIT_NO_PLOT=1` / `OS_TOOLKIT_NO_OPEN=1` | CSV written, plot skipped or generated without opening |
| Documentation | README, knowledge base, `AGENT_CONTEXT.md` and this file updated |

## M1 - Page replacement (Phase B)

Implement FIFO, LRU and LFU over one shared page-reference string and frame
count. Every policy starts with empty frames and its own state, so the
comparison is independent. Report page hits and page faults for each policy and
compare all three on the same input. LFU tie-break: smallest access count, then
the page loaded earliest.

Out of scope for this milestone: per-access frame traces, hit-ratio metrics,
CSV output and charts. Reporting is generalized in M8. Wire menu option 2 and
update the docs.

### M1 checklist

- [x] `include/paging.h` and `src/paging/paging.c` implemented.
- [x] FIFO, LRU and LFU share one reference string but keep independent frames.
- [x] Every policy starts with empty frames.
- [x] Hits and faults reported in one comparison table.
- [x] LFU tie-break documented: smallest count, then earliest loaded page.
- [x] Menu option 2 wired; the stale scheduling-only introduction replaced.
- [x] `read_integer` moved to `common.c` so both modules validate input the
      same way.
- [x] Five regression fixtures added and passing.
- [x] README, knowledge base and test matrix updated.

### Result (verified 2026-10-22)

| Check | Result |
|---|---|
| `make clean && make` | 0 warnings, 0 errors |
| `make test` | `Passed: 14  Failed: 0` |
| Classic string, 3 frames | FIFO 3/9, LRU 2/10, LFU 2/10 (hits/faults) |
| LFU divergence case, 2 frames | LFU 3/3 vs FIFO and LRU 2/4 |
| Invalid input fixture | Re-prompted; final counts match the valid-case fixture |
| Reporting scope | Console only; no page-replacement CSV or chart |

## M2 - Memory allocation (Phase C)

Implement First Fit, Best Fit and Worst Fit over one shared initial block list
and one shared request list, so the three policies are compared fairly. Each
policy starts from a fresh copy of the blocks.

Allocation model (fixed):

- A request is granted exactly its requested size when a block has enough free
  space. The block keeps the remaining free space for later requests.
- Allocated size equals requested size, so internal fragmentation
  (allocated - requested) is zero. No alignment overhead, minimum allocation
  unit or other overhead is introduced.
- External fragmentation is reported as the largest free block.
- A request that no block can satisfy is reported as failed and consumes no
  block space.

Console output, no CSV or chart until M8. Wire menu option 3.

### M2 checklist

- [x] `include/memory.h` and `src/memory/memory.c` implemented.
- [x] First Fit, Best Fit and Worst Fit share the block list and requests but
      use independent copies of the blocks.
- [x] Block splitting: allocation subtracts the request from the chosen block
      and the remainder stays free.
- [x] Failures reported per request, with allocated and failed counts.
- [x] Internal fragmentation computed as allocated - requested (zero under
      exact splitting).
- [x] External fragmentation reported as the largest free block.
- [x] Menu renumbered: option 3 is memory allocation; planned modules, About
      and Exit shifted down; input fixtures updated to exit with 8.
- [x] Five regression fixtures added and passing, including failures, exact
      fits, repeated splitting, policy divergence and invalid input.
- [x] README, knowledge base, test matrix and continuity notes updated.

### Result (verified 2026-10-22)

| Check | Result |
|---|---|
| `make clean && make` | 0 warnings, 0 errors |
| `make test` | `Passed: 19  Failed: 0` (8 scheduling + 5 paging + 5 memory + root override) |
| Classic blocks 100/500/200/300/600, requests 212/417/112/426 | First Fit 3 allocated / 1 failed / largest free 300; Best Fit 4/0/174; Worst Fit 3/1/300 |
| Allocation failures | All requests fail when no block is large enough; blocks unchanged |
| Exact fits and repeated splitting | Remainders reach zero or 100 exactly; internal fragmentation 0 throughout |
| Invalid input | Re-prompted; final report matches the valid exact-fit fixture |
| Reporting scope | Console only; no memory-allocation CSV or chart |

## M3 - POSIX process demo (Phase D)

Demonstrate actual Linux/POSIX system calls, not simulated behavior:

- Process APIs: `fork()`, `exec()` family (`execl()` and `execlp()`), `wait()`,
  `waitpid()`, `getpid()`, `getppid()`.
- File APIs: `open()`, `read()`, `write()`, `close()`, `lseek()`, `stat()`,
  `fcntl()`.
- Directory APIs: `opendir()`, `readdir()`.

All file writes must stay confined to one temporary file created with
`mkstemp()`. Close every descriptor and unlink the temporary file on success
and on every error path. Do not create or modify project files. Report each
system-call failure with `errno`/`perror` and keep the menu usable.

Out of scope: pipes, shared memory, producer-consumer, any other IPC, and any
CSV/chart output. The pipe demonstration belongs to M4. Wire menu option 4.

### M3 checklist

- [x] `include/process_demo.h` and `src/posix/process_demo.c` implemented.
- [x] Parent/child PIDs and PPIDs are obtained from real system calls.
- [x] A child is reaped with `waitpid()` and another with `wait()`.
- [x] A successful `execlp()` replaces a child image; a failed `execl()` path is
      reported and exits with a distinct status.
- [x] The temporary file demonstrates `open`, `write`, `lseek`, `read`,
      `stat`, `fcntl(F_GETFL/F_SETFL)` and `close`.
- [x] `opendir()`/`readdir()` list a real directory and report errors.
- [x] All descriptors are closed and the temporary file is unlinked.
- [x] Menu option 4 wired; menu option 5 remains the planned IPC module.
- [x] Automated check covers markers, cleanup and the failed-exec path.
- [x] README, knowledge base, test matrix and continuity notes updated.

### Result

| Check | Result |
|---|---|
| Build | `make clean && make`: 0 warnings, 0 errors |
| Regression suite | `make test`: `Passed: 20  Failed: 0` |
| Process IDs | Parent, waitpid child, wait child and exec children reported real PIDs; every child PPID matched the parent PID |
| Waiting | `waitpid()` reaped the first child with status 42; `wait()` reaped the second with status 7 |
| Exec success | `execlp()` replaced the child image with `echo`; parent reaped it with status 0 |
| Exec failure | `execl()` on a missing path returned; child reported `ENOENT` and exited 127 as expected |
| File APIs | Temporary file written as `hello world`, read back at offsets 0 and 6, `stat` size 11, `fcntl` toggled/verified `O_APPEND`, descriptor closed, file unlinked |
| Directory APIs | `opendir("/tmp")`, five `readdir()` entries, then `closedir()` |
| Cleanup | Test confirmed the reported temporary path no longer exists and the working directory stayed empty |
| Scope | Console only; no pipes, shared memory, other IPC, CSV or chart output |

## M4 - IPC and synchronization (Phase E)

### Scope

Implement a pthreads + POSIX semaphore bounded-buffer producer-consumer with
configurable buffer size, item count and producer/consumer thread counts, plus a
`fork()` + `pipe()` demonstration. Build with `-pthread`, clean up threads,
semaphores, descriptors and children on every path, and document that these are
demonstrations, not production concurrency frameworks.

Q1 is resolved: shared memory is excluded. Do not add System V or POSIX shared
memory, `mmap()`-backed shared buffers or any other IPC mechanism.

Producer-consumer requirements:

- Bounded ring buffer of integers; capacity, item count and both thread counts
  are read from the user with validation.
- Producers allocate unique item IDs under a mutex; the `empty` and `full`
  semaphores enforce capacity and the buffer mutex protects insertion/removal.
- Consumers reserve items until the item count is exhausted, so every created
  consumer terminates without a fixed quota.
- Verify produced equals consumed, every ID appears exactly once, no duplicates
  or invalid IDs, and the ring is empty after all threads join.
- Join every created thread, destroy every initialized semaphore/mutex and free
  all allocations on success and on every error path.

Pipe requirements:

- Create a pipe with `pipe()`, then `fork()`. The child closes the unused read
  end, writes a fixed message, closes the write end and exits.
- The parent closes the unused write end, reads until `read()` returns 0 (EOF),
  reports the received bytes, reaps the child with `waitpid()` and checks its
  status.
- Check every `pipe`, `fork`, `read`, `write`, `close` and `waitpid` result and
  retry interrupted calls on `EINTR`. Close both ends on every path.

Out of scope: shared memory (Q1), message queues, sockets, signals, any other
IPC mechanism, CSV export and charts. Wire menu option 5.

### M4 checklist

- [x] `include/producer_consumer.h` and `src/posix/producer_consumer.c` implemented.
- [x] Producer-consumer uses pthreads, POSIX semaphores and a bounded buffer.
- [x] Buffer size, item count and thread counts are configurable and validated.
- [x] Integrity check verifies produced/consumed counts, uniqueness and empty ring.
- [x] Pipe demo closes unused ends, observes EOF and reaps the child.
- [x] Error paths and cleanup verified.
- [x] Menu option 5 wired with both demonstrations.
- [x] Regression fixtures and runner extended.
- [x] Warning-free build and `make test` pass.
- [x] README, knowledge base and continuity notes updated.

### Result (verified 2026-10-22)

| Check | Result |
|---|---|
| Build | `make clean && make`: 0 warnings, 0 errors (compiled with `-pthread`) |
| Regression suite | `make test`: `Passed: 24  Failed: 0` (20 previous checks plus 4 IPC cases) |
| Producer-consumer | Real pthreads: 2 producers split 20 items (13/7), 2 consumers split 20 (8/12); integrity line reported `produced=20 consumed=20 duplicates=0 missing=0 invalid=0 buffer_empty=yes` |
| Bounded buffer | Peak occupancy 5 of 5 slots at capacity 5, and 1 of 1 at capacity 1; the semaphores never admitted more items than the ring holds |
| Synchronization stress | 65 runs over five configurations (buffer 1, 3, 5 and 100; up to 8 producers and 8 consumers; 1 to 10000 items) with no hang and no integrity failure |
| Pipe | Child wrote 25 bytes, parent read 25 bytes, `read()` returned 0 for EOF, data matched, `waitpid()` reported exit status 0 |
| Pipe cleanup | Each process closed the unused end; the parent closed both ends after reading and reaped the child |
| Input validation | Submenu, buffer size, item count, producer and consumer counts all rejected out-of-range and non-numeric values and re-prompted; end of input returns to the menu |
| Sanitizers | AddressSanitizer, UndefinedBehaviorSanitizer and LeakSanitizer clean; ThreadSanitizer reported no data races |
| Scope | Pipes, pthreads and POSIX semaphores only; no shared memory, other IPC, CSV or chart output |

### Notes

- Consumers reserve items instead of receiving a fixed quota, so any mix of
  producer and consumer counts terminates without deadlock.
- On a thread-creation failure the module sets a stop flag and forwards
  semaphore tokens, so every already-created thread is released and joined.
- The demonstration has no cancellation, timeout, back-pressure policy or work
  stealing; it is a teaching example, not a production concurrency framework.

## M5 - Deadlock avoidance (Phase F)

Implement Banker's algorithm with allocation, maximum, need and available
matrices. Validate dimensions and reject inconsistent input. Print whether the
state is safe and, when safe, a valid safe sequence.

The module simulates a resource-allocation state. It reads the matrices from the
console, derives `Need = Maximum - Allocation` and runs the safety algorithm with
`Work` starting at `Available`. The lowest-numbered ready process is chosen
first, so the reported sequence is deterministic when several valid sequences
exist.

### M5 checklist

- [x] `include/banker.h` and `src/deadlock/banker.c` implemented.
- [x] Process count, resource count and every matrix value are validated.
- [x] `Need` is derived as `Maximum - Allocation`.
- [x] An entry above the same process's maximum rejects the state before analysis.
- [x] Safety algorithm reports SAFE with a sequence, or UNSAFE without claiming deadlock.
- [x] Menu option 6 wired; options 1-5 unchanged.
- [x] Regression fixtures cover safe, unsafe, multiple-sequence, invalid and zero states.
- [x] Warning-free build and `make test` pass.
- [x] README, knowledge base and continuity notes updated.

### Result (verified 2026-10-22)

| Check | Result |
|---|---|
| Build | `make clean && make`: 0 warnings, 0 errors |
| Regression suite | `make test`: `Passed: 29  Failed: 0` (24 previous checks plus 5 deadlock cases) |
| Safe state | Classic 5-process, 3-resource example reports `P2 -> P4 -> P1 -> P3 -> P5`, a valid safe sequence (the textbook's `P1, P3, P4, P0, P2` is another) |
| Unsafe state | P1 and P2 finish, then no remaining process can finish: `state: UNSAFE`, `unfinished processes: P3` |
| Multiple sequences | A state with several valid orders reports `P1 -> P2 -> P3 -> P4`, the lowest-index-first choice |
| Invalid state | `Allocation P1 R1 = 3` above `Maximum P1 R1 = 2` is rejected before any analysis |
| Input validation | Dimensions, matrix entries and the available vector reject out-of-range and non-numeric values and re-prompt; end of input returns to the menu |
| Limits | 20 processes x 20 resources completes in 20 steps; 1 process x 1 resource with value 999 is handled |
| Sanitizers | AddressSanitizer and UndefinedBehaviorSanitizer clean on all five fixtures plus both limit states |

### Notes

- An unsafe verdict is reported as "unsafe", never as "deadlocked"; the module
  states that no safe completion order is guaranteed for that state.
- Request vectors and resource-release operations are outside Phase F, so the
  module analyzes one static state per run.
- The benchmarking placeholder in the menu was replaced by the Banker's entry;
  benchmarking and the remaining modules received their final numbering in M8.

## M6 - Disk scheduling (Phase G)

Implement FCFS, SSTF and SCAN with one request queue, configurable cylinder
bounds and initial head position, and a documented SCAN direction. Report the
service order and total head movement for each policy. CSV output and charts
were added in M8.

The module reads the cylinder bounds, the initial head position and one shared
request queue from the console; every value is validated against the configured
bounds by `read_integer()`. All three policies run on the same original queue
and head position, so the comparison is fair. Each policy prints its head path
and total head movement, followed by a console comparison table.

SSTF tie-break, fixed before implementation: the nearest cylinder is served
first; on equal distance the lower cylinder number wins; on equal cylinders the
earlier request in the queue wins. SCAN uses a fixed initial direction toward
the upper bound and is conventional SCAN, not LOOK: the arm always reaches the
upper bound before reversing, and that travel is included in the total. The
reverse sweep serves the remaining requests in descending order and ends at the
last one.

### M6 checklist

- [x] `include/disk.h` and `src/disk/disk.c` implemented.
- [x] Cylinder bounds, head position and every request validated and re-prompted.
- [x] One shared queue and head position feed all three policies independently.
- [x] SSTF tie-break documented and deterministic.
- [x] SCAN direction documented; boundary travel included (not LOOK).
- [x] Service order and total head movement reported per policy.
- [x] Console comparison table; no CSV or charts.
- [x] Menu option 7 wired; About moved to 8 and Exit to 9; options 1-6 unchanged.
- [x] Six regression fixtures added and passing.
- [x] Warning-free build and `make test` pass.
- [x] README, knowledge base and continuity notes updated.

### Result (verified 2026-10-22)

| Check | Result |
|---|---|
| Build | `make clean && make`: 0 warnings, 0 errors |
| Regression suite | `make test`: `Passed: 35  Failed: 0` (29 previous checks plus 6 disk cases) |
| Classic queue (head 53, bounds 0-199) | FCFS 640, SSTF 236, SCAN 331 cylinders |
| Requests on one side (head 10, bounds 0-99) | FCFS 40, SSTF 40, SCAN 89; the SCAN total includes the run to cylinder 99, so it is not LOOK |
| Boundary requests | FCFS 149, SSTF 148, SCAN 148 for requests 0 and 99 from head 50 |
| Duplicates and requests at the head | Duplicate requests at the head are served with zero movement; FCFS 30, SSTF 30, SCAN 50 |
| SSTF distance tie | Requests 20 and 80 from head 50: SSTF serves 20 first (lower cylinder), FCFS keeps queue order |
| Invalid values | Non-numeric and out-of-range bounds, head and requests re-prompt; the final state matches a clean run |
| Independent verification | A separate Python model matched every path and total on all six fixtures, 300 random states and 5 targeted edge states |
| Sanitizers | AddressSanitizer and UndefinedBehaviorSanitizer clean on the six fixtures plus 200-request, maximum-bound and all-at-head states |

### Notes

- The menu now has nine entries: disk scheduling is 7, About moved to 8 and
  Exit to 9. Every fixture exits with 9; the three invalid-input fixtures feed
  `10` as the out-of-range menu value.
- The reverse SCAN sweep stops at the last request, matching the textbook
  example; a second run to the lower bound would add travel that serves no
  request.
- The five deterministic fixtures compare the whole report block; the invalid
  case uses a marker checklist so the rejection messages themselves are
  asserted.

## M7 - File allocation (Phase H)

Implement contiguous, linked and indexed allocation over a fixed simulated disk;
allocate, delete and retrieve files; report block usage, allocation failures and
wasted space. Output is console only, and the module never reads or modifies a
real filesystem.

The disk model was fixed before implementation:

- The disk has 1-128 blocks of 8-4096 bytes each. Every method starts from an
  independent copy of the same empty disk and receives the same operation list.
- File sizes are entered in bytes. A file identifier is 1-32 and a run contains
  1-64 operations (allocate, delete, retrieve).
- Contiguous allocation needs `ceil(size / block_size)` data blocks and takes
  the first free run that fits.
- Linked allocation reserves a 4-byte next pointer in every data block, so
  `block_size - 4` bytes per block carry file data. It needs
  `ceil(size / (block_size - 4))` blocks and takes the lowest free blocks in
  chain order.
- Indexed allocation needs `ceil(size / block_size)` data blocks plus exactly one
  index block. The index block holds `block_size / 4` 4-byte pointers; a file
  that needs more data blocks than that is rejected. The index block is
  allocated first, then the data blocks are taken in ascending block order.
- Allocation is atomic: every capacity and free-space check runs before the
  disk changes, so a failure leaves the volume untouched. Deletion frees all
  data and index blocks. Retrieval prints the file size and the stored block
  mapping (plus the index block for indexed files).
- Wasted space is `allocated block bytes - requested bytes` over the active
  files. It therefore includes last-block slack, linked pointer slots and index
  blocks. Index blocks are also counted as metadata blocks.

### M7 checklist

- [x] `include/file_alloc.h` and `src/file_alloc/file_alloc.c` implemented.
- [x] Disk model, file-size representation, first-fit rule and metadata
  assumptions documented before implementation.
- [x] Contiguous, linked and indexed allocation with allocation, deletion and
  retrieval.
- [x] Each method runs on an independent volume copy with the same operations.
- [x] Block count, block size, operation count, operation type, file id and file
  size validated and re-prompted.
- [x] Block usage, failures and wasted space displayed per method.
- [x] Interface and documentation state that no real filesystem is touched.
- [x] Menu option 8 wired; About moved to 9 and Exit to 10; options 1-7 unchanged.
- [x] Seven regression fixtures added and passing.
- [x] Warning-free build and `make test` pass.
- [x] README, knowledge base and continuity notes updated.

### Result (verified 2026-10-23)

| Check | Result |
|---|---|
| Build | `make clean && make`: 0 warnings, 0 errors |
| Regression suite | `make test`: `Passed: 42  Failed: 0` (35 previous checks plus 7 file allocation cases) |
| Mixed operations | Contiguous keeps files 3 and 4 in blocks [0 1] and [2 3 4]; linked uses [0 1] and [2 3 4 5]; indexed uses index 0/data [1 2] and index 3/data [4 5 6]; wasted 30/94/158 bytes |
| No space | With 4 blocks of 64 bytes, contiguous allocates the 250-byte file and rejects the next one; linked and indexed reject the 250-byte file (5 blocks needed) and allocate the 64-byte file instead |
| Deletion and reuse | File 1's blocks are freed and file 3 reuses them in every method; file 2's mapping is unchanged |
| Contiguous fragmentation | Ten one-block files then delete files 1, 3 and 5: the 200-byte request fails with `no contiguous run of 2 blocks (3 free)`, while linked stores it in [0 1 4] and indexed in index 0/data [1 4] |
| Indexed capacity limit | With 8-byte blocks the index holds 2 pointers; the 24-byte file (3 data blocks) is rejected, the 8-byte file uses index 0/data [1] |
| Minimum disk | One 8-byte block: contiguous and linked store a 4-byte file, indexed rejects it because an index block plus data needs 2 blocks; deletion then reuse succeeds |
| Invalid input | Out-of-range menu, block count, block size, operation count, operation type, file id and file size all re-prompt; the final run matches a clean run |
| Independent verification | A separate Python model matched every event, mapping, used/free set and waste total on the six deterministic fixtures |
| Sanitizers | AddressSanitizer and UndefinedBehaviorSanitizer clean on all seven fixtures and a 128-block maximum-bound run |

### Notes

- The menu now has ten entries: file allocation is 8, About moved to 9 and
  Exit to 10. Every fixture exits with 10; the three older invalid-input
  fixtures and the new file allocation invalid fixture feed `11` as the
  out-of-range menu value.
- The six deterministic file fixtures compare the whole report block; the
  invalid-input case uses a marker checklist so every rejection message and the
  final clean mapping are asserted.
- The contiguous fragmentation fixture proves the request fails even though the
  total free space (3 blocks) exceeds the two blocks required, because no run of
  two exists.
- CSV export, charts and generalized benchmarking were added in M8.

## M8 - Benchmarking and final integration (Phase I)

Generalize reporting without changing algorithm results: extract the shared CSV
writer and plot driver, define one CSV convention while keeping module-specific
metrics, generalize the plotting script, integrate the five algorithm-comparison
modules, and verify the whole application from a clean tree. Simulated OS metrics
stay simulated; no wall-clock or `CLOCK_MONOTONIC` runtime timing is added (B5
remains out of scope).

### Tasks

- [x] M8.1 Implement the shared reporting layer in `include/benchmark.h`,
      `src/benchmark/benchmark.c` and `src/benchmark/csv_logger.c`.
- [x] M8.2 Define and document the CSV convention (see the checklist).
- [x] M8.3 Integrate reporting in CPU scheduling, page replacement, memory
      allocation, disk scheduling and file allocation; keep Banker's algorithm,
      the POSIX demonstration and the IPC demonstration console-only.
- [x] M8.4 Generalize `scripts/plot_results.py` to five modules with per-module
      labels and validation while preserving the CPU chart.
- [x] M8.5 Report CSV write failures and plot failures without discarding the
      algorithm results.
- [x] M8.6 Update the Makefile with the benchmark sources and verify the flags.
- [x] M8.7 Add reporting, menu-dispatch, CSV-failure and plot-failure tests.
- [x] M8.8 Verify a clean-tree build and test; confirm no generated artifact is
      tracked.
- [x] M8.9 Update README, knowledge base, continuity notes and test matrix; write
      the final report.

### M8 checklist

- [x] The shared CSV writer opens `data/results/<module>.csv` under the project
      root, escapes commas/quotes/newlines, and removes a partial file when a
      write or close fails.
- [x] CSV convention: `algorithm` is the first column, metrics are snake_case,
      workload-context columns follow the metrics, floating-point values use two
      decimals, and each file holds one row per algorithm. No timestamp or
      runtime column.
- [x] The CPU scheduling CSV keeps its exact columns and values.
- [x] Page replacement, memory allocation, disk scheduling and file allocation
      write their module-specific metrics through the same writer.
- [x] Banker's algorithm, the POSIX demonstration and the IPC demonstration stay
      console-only; no artificial comparison metrics were invented.
- [x] `scripts/plot_results.py` accepts `cpu_scheduling`, `page_replacement`,
      `memory_allocation`, `disk_scheduling` and `file_allocation`, validates the
      CSV, and exits 1 with a clear message on bad input.
- [x] Plot generation prefers `.venv/bin/python` and falls back to `python3`; a
      missing interpreter, missing Matplotlib or missing script prints a failure
      message and leaves the CSV and results valid.
- [x] `OS_TOOLKIT_NO_PLOT` skips plotting and `OS_TOOLKIT_NO_OPEN` skips the
      viewer, as before.
- [x] The Makefile compiles all 12 sources, including both benchmark files, with
      `-Wall -Wextra -Wpedantic -std=c11 -pthread -Iinclude`.
- [x] `tests/run_tests.sh` adds five reporting comparisons, one menu-dispatch
      chain, two CSV-failure cases and two plot-failure cases.
- [x] `tests/test_cases.md` documents the new checks and CSV schemas.
- [x] A tree without `build/`, `data/results/`, `.git/`, `.venv/` and
      `__pycache__/` builds and passes the full suite.
- [x] `git ls-files` lists no `build/` or `data/` path.

### Result (verified 2026-10-23)

| Check | Result |
|---|---|
| Build | `make clean && make`: 0 warnings, 0 errors |
| Regression suite | `make test`: `Passed: 52  Failed: 0` (42 previous checks plus 10 new: 5 reporting, 1 menu dispatch, 2 CSV-failure, 2 plot-failure) |
| CSV schemas | `cpu_scheduling.csv` unchanged; `page_replacement.csv` (hits, faults, frame count, reference count); `memory_allocation.csv` (allocated/failed requests, fragmentation, largest free block, block and request counts); `disk_scheduling.csv` (total head movement, request count, initial head, cylinder bounds); `file_allocation.csv` (block usage, operation counters, bytes, waste, metadata and geometry) |
| Charts | Five PNGs generated with system Python 3 and Matplotlib 3.10.8 (no `.venv`): CPU 2381x787, page replacement 1741x662, memory allocation 1742x1320, disk scheduling 862x661, file allocation 1742x1320. All values match the CSVs; the CPU chart is unchanged |
| Plot script errors | Unknown module, missing CSV, missing column, invalid value and empty rows each exit 1 with a clear message |
| CSV failure | A blocked `data` path prints the save failure, keeps the algorithm report and exits 0; no plot is attempted |
| Plot failure | A missing plotting script prints `Could not generate the graph.` and states that the CSV and the algorithm results are still valid; exit 0 |
| Menu dispatch | One process runs choices 1-8, About (9), an invalid choice (11) and Exit (10); every module banner and exactly five CSV save messages appear |
| Clean tree | A copy excluding `build/`, `data/results/`, `.git/`, `.venv/` and `__pycache__/` builds and passes 52/52 |
| Repository hygiene | `git ls-files` lists no `build/` or `data/` path; generated outputs are git-ignored |

### Notes

- The `CLOCK_MONOTONIC` timer mentioned in an earlier draft of this section was
  not implemented. The knowledge base asks for reporting of simulated
  comparison metrics, and wall-clock runtime benchmarking is backlog item B5;
  adding it would have changed the approved scope. Every CSV value is a
  simulated metric, never an implementation runtime.
- The reporting tests reuse the existing deterministic workloads instead of
  adding new input fixtures, so the suite stays focused on integration.
- A real `git clone` would omit the uncommitted M0-M8 changes, so the clean-tree
  check copies the working tree without generated files, `.git/` and `.venv/`,
  builds it and runs the suite there.
- The plot driver prints `Trying system Python...` whenever it falls back to
  the system interpreter, even when no `.venv` exists. It is cosmetic.
- The About screen (menu option 9) now names the reporting scope and states that
  benchmark metrics are simulated algorithm results, not runtime timings.

## Open questions

| ID | Question | Needed by | Status |
|---|---|---|---|
| Q1 | Is shared memory part of the final IPC scope? The status table said "Pipes/shared memory plus pthreads and semaphores", while Phase E said "Shared memory if included in the final scope". | M4 | Resolved 2026-10-22: shared memory is excluded. Phase E uses pipes and pthreads with POSIX semaphores only. The knowledge base, status table and Phase E text were aligned to this decision. |

## Backlog (out of scope, not scheduled)

These items are not part of the knowledge-base scope and must not be
implemented under the current roadmap.

- B1: Preemptive SRTF and preemptive Priority, priority aging.
- B2: Multi-workload comparison.
- B3: Per-process CSV rows.
- B4: Gantt output with idle gaps.
- B5: Wall-clock runtime benchmarking, kept distinct from simulated metrics.
- B6: Config-file and batch input.
- B7: Versioned sample datasets.
- B8: Optional GitHub Actions CI.

## Progress log

| Date | Milestone | Change | Verification |
|---|---|---|---|
| 2026-10-22 | - | Repository audited against the knowledge base; plan approved (full roadmap) | Build, sample workload and file inventory verified |
| 2026-10-22 | M0 | Plan and continuity knowledge base created | n/a |
| 2026-10-22 | M0 | Warning fix, `common.c` path helper, robust plot launch, dead code removed, `.gitignore`, regression suite, `make test`, docs synced | 0 warnings; `make test` 9/9; run from outside the repo verified |
| 2026-10-22 | M1 | Knowledge base confirmed as the authoritative scope; roadmap narrowed (no module CSV/charts before M8, no backlog work); M1 started | n/a |
| 2026-10-22 | M1 | FIFO/LRU/LFU page replacement, menu option 2, five fixtures, shared `read_integer`, docs synced | 0 warnings; `make test` 14/14 |
| 2026-10-22 | M2 | First/Best/Worst Fit with exact splitting, fragmentation and failure reporting, menu option 3, five fixtures, docs synced | 0 warnings; `make test` 19/19 |
| 2026-10-22 | M3 | Real fork/exec/wait/waitpid, PID reporting, open/read/write/close/lseek/stat/fcntl, opendir/readdir, temporary-file cleanup, menu option 4, fixture and docs synced | 0 warnings; `make test` 20/20; failed-exec and cleanup checks passed |
| 2026-10-22 | M4 | pthreads + POSIX semaphore producer-consumer with integrity check, fork/pipe demo, menu option 5, four fixtures, docs synced | 0 warnings; `make test` 24/24; 65-run stress pass; sanitizers clean |
| 2026-10-22 | M5 | Banker's algorithm with Need derivation, rejection of inconsistent states, safety analysis and safe sequence, menu option 6, five fixtures, docs synced | 0 warnings; `make test` 29/29; limit states and sanitizers clean |
| 2026-10-22 | M6 | FCFS/SSTF/SCAN over one queue and head with configurable bounds, documented SSTF tie-break and SCAN direction, menu option 7, six fixtures, docs synced | 0 warnings; `make test` 35/35; independent model over 300 random states; sanitizers clean |
| 2026-10-23 | M7 | Contiguous/linked/indexed allocation on independent simulated volumes, validation, deletion/reuse, fragmentation and capacity failures, waste reporting, menu option 8, seven fixtures, docs synced | 0 warnings; `make test` 42/42; independent model on six fixtures; sanitizers clean |
| 2026-10-23 | M8 | Shared CSV writer (`benchmark.h`, `benchmark.c`, `csv_logger.c`), reporting in five comparison modules, generalized plot script, non-fatal CSV/plot failures, validated menu dispatch, 10 new checks, docs synced | 0 warnings; `make test` 52/52; all five charts generated and inspected; clean-tree copy builds and passes 52/52; no generated file tracked |

## Decision log

| Date | Decision | Rationale |
|---|---|---|
| 2026-10-22 | Follow the knowledge-base order M0 to M8 | Matches the documented roadmap and team ownership |
| 2026-10-22 | Fix warnings by passing an algorithm index to `print_results` | Verified: 0 warnings with identical output; no diagnostic masking |
| 2026-10-22 | Resolve paths from `/proc/self/exe` with an `OS_TOOLKIT_ROOT` override | Linux target; keeps the binary location convention `build/os_toolkit` |
| 2026-10-22 | Regression tests compare the generated CSV | Deterministic, covers all four algorithms, independent of console formatting |
| 2026-10-22 | Add `OS_TOOLKIT_NO_PLOT` in addition to `OS_TOOLKIT_NO_OPEN` | Lets `make test` run without Matplotlib while still exercising CSV export |
| 2026-10-22 | Leave line endings unchanged (working tree CRLF, index LF) | Avoids a repository-wide whitespace diff; review with `git diff --ignore-cr-at-eol` |
| 2026-10-22 | Follow `PROJECT_KNOWLEDGE_BASE.md` where it conflicts with this plan | The knowledge base is the authoritative specification |
| 2026-10-22 | Defer all module CSV and charts to M8 | Avoids module-specific reporting that would be replaced by the generalized layer |
| 2026-10-22 | Do not implement B1-B8 | The backlog is outside the knowledge-base scope |
| 2026-10-22 | Every paging policy starts with empty frames | Phase B asks for a fair independent comparison |
| 2026-10-22 | LFU tie-break: smallest count, then earliest loaded | Deterministic and documented in code, tests and docs |
| 2026-10-22 | No paging CSV or chart in M1 | General reporting arrives in M8; avoids throwaway module reporting |
| 2026-10-22 | Memory allocation splits blocks exactly; allocated = requested | The user required this model; no alignment or minimum unit is invented, so internal fragmentation is genuinely zero |
| 2026-10-22 | External fragmentation = largest free block | Defined by the approved scope and this plan |
| 2026-10-22 | Insert memory allocation at menu option 3 and shift the rest | Keeps implemented modules together; all fixtures now exit with 8 |
| 2026-10-22 | Memory tests compare the full module report | Verifies block choices, remainders, fragmentation and failures, not only summary counts |
| 2026-10-22 | POSIX demo uses real system calls only | The knowledge base requires actual POSIX behavior; output is not simulated or hardcoded |
| 2026-10-22 | All POSIX file writes stay in one `mkstemp()` file | Satisfies the approved scope and keeps project files untouched; the file is unlinked on every path |
| 2026-10-22 | POSIX regression uses a marker checklist plus PID/cleanup assertions | PIDs, temporary names and directory entries are dynamic, so exact-output comparison would be invalid |
| 2026-10-22 | Menu option 4 is POSIX; option 5 remains planned IPC | Implements the module without moving unrelated planned entries |
| 2026-10-22 | Do not commit until the whole project is implemented and explicitly authorized | The user requires a complete implementation before any GitHub commit |
| 2026-10-22 | SSTF tie-break: nearest cylinder, then lower cylinder number, then earlier queue entry | Fixed before implementation so ties never depend on scan order |
| 2026-10-22 | SCAN starts toward the upper bound and always reaches it before reversing | Requirement: conventional SCAN with boundary travel, explicitly not LOOK |
| 2026-10-22 | Disk scheduling at menu option 7; About 8, Exit 9; fixtures exit with 9 | Keeps module options 1-6 unchanged; the invalid fixtures feed 10 as the out-of-range value |
| 2026-10-22 | Deterministic disk fixtures compare the full report block | Paths, totals and the comparison table are reproducible; only the invalid case needs a marker checklist |
| 2026-10-23 | File allocation uses a fixed simulated disk of 1-128 blocks of 8-4096 bytes | The size range makes block counts small enough to verify by hand while still exercising capacity limits |
| 2026-10-23 | Files are sized in bytes; first-fit allocation in all three methods | A single representation makes the three methods directly comparable; first-fit is deterministic and easy to verify |
| 2026-10-23 | Linked blocks reserve a 4-byte next pointer | Link overhead is represented explicitly instead of being silently excluded; it changes block counts and wasted space |
| 2026-10-23 | Indexed files use one index block with `block_size / 4` pointers | Capacity is derived from the block size and enforced consistently; over-capacity files fail instead of using an undefined multi-level scheme |
| 2026-10-23 | File allocation at menu option 8; About 9, Exit 10; fixtures exit with 10 | Keeps module options 1-7 unchanged; the invalid fixtures feed 11 as the out-of-range value |
| 2026-10-23 | Deterministic file fixtures compare the full report block | Mappings, used/free sets, failures and waste totals are reproducible; the invalid case uses markers |
| 2026-10-23 | Reporting covers only the five algorithm-comparison modules | Deadlock avoidance, POSIX and IPC have no comparable alternative algorithms, so CSV metrics for them would be invented data |
| 2026-10-23 | No wall-clock or `CLOCK_MONOTONIC` runtime timing | Simulated metrics (waiting time, faults, head movement, waste) are not implementation runtimes; runtime benchmarking remains B5 and out of scope |
| 2026-10-23 | One CSV convention: algorithm first, snake_case metrics, workload context last, two-decimal values | A single shape keeps the writer and plot script generic while module-specific columns stay intact |
| 2026-10-23 | CSV and plot failures are reported without failing the algorithm run | The comparison result is the deliverable; an unwritable path or missing Matplotlib must not discard it |
| 2026-10-23 | Plot output stays `<module>_comparison.png` under `data/results/` | Preserves the existing CPU artifact names and layout while generalizing the script |
| 2026-10-23 | Reporting tests reuse existing workloads; failures are injected with an isolated `OS_TOOLKIT_ROOT` | Avoids duplicate fixtures and makes both failure paths deterministic |

## Approval record

- 2026-10-22: Full roadmap approved (M0 through M8). The user requested this
  plan file and a continuity knowledge base before implementation began.
