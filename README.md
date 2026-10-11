
# Integrated OS Toolkit and Benchmarking Suite

A modular Operating Systems project implemented in C for Linux/POSIX.

## Objectives

- Simulate classical operating-system algorithms.
- Demonstrate Linux/POSIX system calls and inter-process communication.
- Implement concurrent workloads using pthreads and semaphores.
- Benchmark algorithms and compare their results using CSV files and charts.

## Initial Implementation

- [x] Project structure and CLI menu
- [ ] FCFS and Round Robin CPU scheduling
- [ ] FIFO and LRU page replacement
- [ ] POSIX process demonstration
- [ ] Producer-consumer synchronization
- [ ] CSV benchmark logging
- [ ] Comparative charts

## Technologies

- C and GCC
- Linux/POSIX APIs
- POSIX threads and semaphores
- Make
- Python, pandas and matplotlib for visualization

## Build and Run

```bash
make
./build/os_toolkit
```

## Project Status

Initial implementation in progress.

## How to Run the Project

1. **Clone the repository**
   ```bash
   git clone https://github.com/adityaDev-2005/Integrated-OS-Toolkit.git
   cd Integrated-OS-Toolkit
   ```

2. **Install dependencies**
   ```bash
   sudo apt update
   sudo apt install build-essential python3 python3-venv
   ```

3. **Set up the Python environment and install Matplotlib**
   ```bash
   python3 -m venv .venv
   .venv/bin/python -m pip install matplotlib
   ```

4. **Compile the project**
   ```bash
   make clean && make
   ```

5. **Run the application**
   ```bash
   ./build/os_toolkit
   ```

6. **Run CPU Scheduling Comparison**

   Select option `1` from the main menu. Enter the number of processes, their arrival times, burst times, and priorities, followed by the Round Robin time quantum.

7. **View the results**

   The application automatically runs FCFS, SJF (non-preemptive), Priority (non-preemptive), and Round Robin. It displays execution timelines, process metrics, and a comparison table.

8. **View the generated outputs**

   After the comparison, the application saves the numerical results to `data/results/cpu_scheduling.csv` and generates the comparison graph at `data/results/cpu_scheduling_comparison.png`. The graph is opened automatically when the desktop environment supports it.

   To inspect the generated files:
   ```bash
   ls -lh data/results/
   ```

**Note:** The CSV file and graph are generated locally and do not need to be committed to the GitHub repository. Run the application again with a new workload to generate updated results.
