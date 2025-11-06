// main.cpp
// Linux-Based Process Scheduler Simulation
// Supports: Round Robin (RR) and Preemptive Priority Scheduling (PPS)
// Compile: g++ -std=c++17 -O2 main.cpp -o scheduler

#include <bits/stdc++.h>
using namespace std;

struct Process {
    int pid;
    int arrival;
    int burst;
    int priority; // lower number -> higher priority
    int remaining;
    int start_time;
    int completion_time;
    int waiting_time;
    int turnaround_time;
    Process(int id=0,int a=0,int b=0,int p=0){
        pid=id; arrival=a; burst=b; priority=p;
        remaining=b; start_time=-1; completion_time=0;
        waiting_time=0; turnaround_time=0;
    }
};

struct GanttEntry {
    int pid; int start; int end;
    GanttEntry(int p,int s,int e):pid(p),start(s),end(e){}
};

void print_metrics(const vector<Process>& procs, const vector<GanttEntry>& gantt, int total_time){
    int n = procs.size();
    double sum_wt=0, sum_tat=0;
    for(auto &p: procs){
        sum_wt += p.waiting_time;
        sum_tat += p.turnaround_time;
    }
    double avg_wt = sum_wt/n;
    double avg_tat = sum_tat/n;
    int cpu_busy=0;
    for(auto &g: gantt) if(g.pid!=-1) cpu_busy += (g.end - g.start);
    double cpu_util = 100.0 * cpu_busy / (double) total_time;
    double throughput = (double)n / total_time;
    int context_switches = 0;
    for(size_t i=1;i<gantt.size();++i) if(gantt[i].pid != gantt[i-1].pid) context_switches++;

    cout << fixed << setprecision(2);
    cout << "\n--- Metrics ---\n";
    cout << "Total time (makespan): " << total_time << "\n";
    cout << "Average Waiting Time : " << avg_wt << "\n";
    cout << "Average Turnaround Time : " << avg_tat << "\n";
    cout << "CPU Utilization: " << cpu_util << " %\n";
    cout << "Throughput (processes/unit time): " << throughput << "\n";
    cout << "Context switches (approx): " << context_switches << "\n";
}

void print_gantt(const vector<GanttEntry>& gantt){
    cout << "\nGantt Chart (pid : [start -> end])\n";
    for(auto &g: gantt){
        if(g.pid == -1) cout << "idle" << " : [" << g.start << " -> " << g.end << "]  ";
        else cout << "P" << g.pid << " : [" << g.start << " -> " << g.end << "]  ";
    }
    cout << "\n";
}

// Round Robin (quantum) - preemptive by design
void round_robin(vector<Process> procs, int quantum){
    cout << "\n=== Round Robin (quantum = " << quantum << ") ===\n";
    int n = procs.size();
    // sort by arrival then pid
    sort(procs.begin(), procs.end(), [](const Process& a, const Process& b){
        if(a.arrival!=b.arrival) return a.arrival < b.arrival;
        return a.pid < b.pid;
    });

    queue<int> q; // indices into procs
    vector<bool> in_queue(n,false);
    int time = 0;
    int completed = 0;
    vector<GanttEntry> gantt;

    // push initial arrivals at time 0
    for(int i=0;i<n && procs[i].arrival==0;i++){
        q.push(i); in_queue[i]=true;
    }
    int idx_next_arrival = 0;
    while(idx_next_arrival < n && procs[idx_next_arrival].arrival==0) idx_next_arrival++;

    // if no process arrived at time 0, fast-forward to first arrival (idle)
    if(q.empty() && idx_next_arrival < n){
        int next_t = procs[idx_next_arrival].arrival;
        gantt.emplace_back(-1, time, next_t);
        time = next_t;
        while(idx_next_arrival < n && procs[idx_next_arrival].arrival==time){
            q.push(idx_next_arrival); in_queue[idx_next_arrival]=true; idx_next_arrival++;
        }
    }

    while(completed < n){
        if(q.empty()){
            if(idx_next_arrival < n){
                int next_t = procs[idx_next_arrival].arrival;
                if(time < next_t){
                    gantt.emplace_back(-1, time, next_t);
                    time = next_t;
                }
                while(idx_next_arrival < n && procs[idx_next_arrival].arrival==time){
                    q.push(idx_next_arrival); in_queue[idx_next_arrival]=true; idx_next_arrival++;
                }
                continue;
            }
            break;
        }

        int i = q.front(); q.pop(); in_queue[i]=false;
        Process &p = procs[i];
        if(p.start_time == -1) p.start_time = time;
        int exec = min(quantum, p.remaining);
        gantt.emplace_back(p.pid, time, time+exec);
        p.remaining -= exec;
        // advance time and add arrivals that come while running this quantum
        int old_time = time;
        time += exec;
        while(idx_next_arrival < n && procs[idx_next_arrival].arrival <= time){
            q.push(idx_next_arrival); in_queue[idx_next_arrival]=true; idx_next_arrival++;
        }
        if(p.remaining > 0){
            q.push(i); in_queue[i]=true;
        } else {
            p.completion_time = time;
            p.turnaround_time = p.completion_time - p.arrival;
            p.waiting_time = p.turnaround_time - p.burst;
            completed++;
        }
    }

    // finalize metrics: copy back waiting/turnaround to a vector for printing
    // Procs vector already has it
    print_gantt(gantt);
    print_metrics(procs, gantt, gantt.empty() ? 0 : gantt.back().end);

    // print table
    cout << "\nPID  Arrival  Burst  Priority  Start  Completion  Waiting  Turnaround\n";
    for(auto &p: procs){
        cout << setw(3) << p.pid << setw(9) << p.arrival << setw(7) << p.burst
             << setw(9) << p.priority << setw(7) << p.start_time << setw(11) << p.completion_time
             << setw(9) << p.waiting_time << setw(11) << p.turnaround_time << "\n";
    }
}

// Preemptive Priority Scheduling (smaller priority value => higher priority)
void preemptive_priority(vector<Process> procs){
    cout << "\n=== Preemptive Priority Scheduling (lower - higher priority) ===\n";
    int n = procs.size();
    sort(procs.begin(), procs.end(), [](const Process& a, const Process& b){
        if(a.arrival!=b.arrival) return a.arrival < b.arrival;
        if(a.priority!=b.priority) return a.priority < b.priority;
        return a.pid < b.pid;
    });

    int time = 0;
    int completed = 0;
    int idx = 0;
    // min-heap by priority then arrival then pid
    auto cmp = [&](int a, int b){
        if(procs[a].priority != procs[b].priority) return procs[a].priority > procs[b].priority;
        if(procs[a].arrival != procs[b].arrival) return procs[a].arrival > procs[b].arrival;
        return procs[a].pid > procs[b].pid;
    };
    priority_queue<int, vector<int>, decltype(cmp)> pq(cmp);
    vector<GanttEntry> gantt;

    // advance to first arrival if needed
    if(idx < n && procs[idx].arrival > time){
        gantt.emplace_back(-1, time, procs[idx].arrival);
        time = procs[idx].arrival;
    }

    while(completed < n){
        while(idx < n && procs[idx].arrival <= time){
            pq.push(idx); idx++;
        }

        if(pq.empty()){
            if(idx < n){
                int next_t = procs[idx].arrival;
                if(time < next_t){
                    gantt.emplace_back(-1, time, next_t);
                    time = next_t;
                }
                continue;
            }
            break;
        }

        int cur = pq.top(); pq.pop();
        Process &p = procs[cur];
        if(p.start_time == -1) p.start_time = time;
        // run for 1 unit (time quantum of 1) to allow preemption checks
        int run_for = 1;
        gantt.emplace_back(p.pid, time, time+run_for);
        p.remaining -= run_for;
        time += run_for;

        // push any new arrivals that come at this time
        while(idx < n && procs[idx].arrival <= time){
            pq.push(idx); idx++;
        }

        if(p.remaining > 0){
            pq.push(cur); // still has remaining, may be preempted by higher priority
        } else {
            p.completion_time = time;
            p.turnaround_time = p.completion_time - p.arrival;
            p.waiting_time = p.turnaround_time - p.burst;
            completed++;
        }
    }

    print_gantt(gantt);
    print_metrics(procs, gantt, gantt.empty() ? 0 : gantt.back().end);
    cout << "\nPID  Arrival  Burst  Priority  Start  Completion  Waiting  Turnaround\n";
    for(auto &p: procs){
        cout << setw(3) << p.pid << setw(9) << p.arrival << setw(7) << p.burst
             << setw(9) << p.priority << setw(7) << p.start_time << setw(11) << p.completion_time
             << setw(9) << p.waiting_time << setw(11) << p.turnaround_time << "\n";
    }
}

vector<Process> load_sample(){ // a small helper that creates sample processes
    // You can replace these or read from file as shown in README
    vector<Process> v;
    v.emplace_back(1, 0, 5, 2);
    v.emplace_back(2, 1, 3, 1);
    v.emplace_back(3, 2, 8, 4);
    v.emplace_back(4, 3, 6, 3);
    return v;
}

int main(int argc, char** argv){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "Linux-Based Process Scheduler Simulation\n";
    cout << "Usage: ./scheduler [mode] [quantum]\n";
    cout << "Modes: rr (Round Robin), pps (Preemptive Priority Scheduling)\n";
    cout << "If no args provided, sample dataset will run both algorithms.\n";

    vector<Process> procs;
    if(argc >= 2){
        string mode = argv[1];
        // read processes from stdin (convenient for piping from file)
        // expected input format (first line n):
        // n
        // pid arrival burst priority
        // ...
        int n;
        if(!(cin >> n)){
            cerr << "Expected input: first line = n (number of processes) followed by lines: pid arrival burst priority\n";
            return 1;
        }
        procs.reserve(n);
        for(int i=0;i<n;i++){
            int pid,a,b,p;
            cin >> pid >> a >> b >> p;
            procs.emplace_back(pid,a,b,p);
        }

        if(mode == "rr"){
            int quantum = 2;
            if(argc >= 3) quantum = stoi(argv[2]);
            round_robin(procs, quantum);
        } else if(mode == "pps"){
            preemptive_priority(procs);
        } else {
            cerr << "Unknown mode: " << mode << "\n";
            return 1;
        }
    } else {
        // run sample
        procs = load_sample();
        round_robin(procs, 2);
        for(auto &p: procs){ p.remaining = p.burst; p.start_time=-1; p.completion_time=0; p.waiting_time=0; p.turnaround_time=0;}
        preemptive_priority(procs);
    }

    return 0;
}
