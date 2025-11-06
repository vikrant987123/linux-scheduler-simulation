# Linux-Based Process Scheduler Simulation

A C++ simulator for learning OS scheduling algorithms.  
Implements:
- **Round Robin (RR)** (quantum-based, preemptive)
- **Preemptive Priority Scheduling (PPS)**

Shows:
- Gantt-chart style timeline
- Average waiting time, average turnaround time
- CPU utilization, throughput, context switches
- Per-process start/completion/waiting/turnaround times

---

## Files
- `main.cpp` - C++17 source
- `README.md` - this file

---

## Build
Requires a modern g++ with C++17 support.

```bash
g++ -std=c++17 -O2 main.cpp -o scheduler
