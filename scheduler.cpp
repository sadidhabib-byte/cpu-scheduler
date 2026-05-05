#include "scheduler.hpp"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <queue>

namespace {

void calculate_times(std::vector<Process>& p) {
    for (auto& pr : p) {
        pr.tat = pr.ct - pr.at;
        pr.wt = pr.tat - pr.bt;
    }
}

ScheduleResult finalize(std::vector<Process> p, std::vector<int> gantt) {
    calculate_times(p);
    ScheduleResult r;
    r.processes = std::move(p);
    r.gantt = std::move(gantt);
    if (!r.processes.empty()) {
        float swt = 0, stat = 0;
        for (const auto& pr : r.processes) {
            swt += static_cast<float>(pr.wt);
            stat += static_cast<float>(pr.tat);
        }
        const float n = static_cast<float>(r.processes.size());
        r.avg_wt = swt / n;
        r.avg_tat = stat / n;
    }
    return r;
}

}  // namespace

ScheduleResult schedule_fcfs(std::vector<Process> processes) {
    std::sort(processes.begin(), processes.end(),
              [](const Process& a, const Process& b) { return a.at < b.at; });

    int time = 0;
    std::vector<int> gantt;

    for (auto& pr : processes) {
        while (time < pr.at) {
            gantt.push_back(-1);
            ++time;
        }
        for (int i = 0; i < pr.bt; ++i) {
            gantt.push_back(pr.pid);
            ++time;
        }
        pr.ct = time;
    }

    return finalize(std::move(processes), std::move(gantt));
}

ScheduleResult schedule_sjf(std::vector<Process> processes) {
    const int n = static_cast<int>(processes.size());
    int completed = 0;
    int time = 0;
    std::vector<int> gantt;

    while (completed < n) {
        int idx = -1;
        int mn = INT_MAX;
        for (int i = 0; i < n; ++i) {
            if (processes[i].at <= time && processes[i].ct == 0 && processes[i].bt < mn) {
                mn = processes[i].bt;
                idx = i;
            }
        }

        if (idx == -1) {
            gantt.push_back(-1);
            ++time;
        } else {
            for (int i = 0; i < processes[idx].bt; ++i) {
                gantt.push_back(processes[idx].pid);
                ++time;
            }
            processes[idx].ct = time;
            ++completed;
        }
    }

    return finalize(std::move(processes), std::move(gantt));
}

ScheduleResult schedule_srtf(std::vector<Process> processes) {
    const int n = static_cast<int>(processes.size());
    int time = 0;
    int completed = 0;
    std::vector<int> gantt;

    for (auto& pr : processes) pr.rt = pr.bt;

    while (completed < n) {
        int idx = -1;
        int mn = INT_MAX;

        for (int i = 0; i < n; ++i) {
            if (processes[i].at <= time && processes[i].rt > 0 && processes[i].rt < mn) {
                mn = processes[i].rt;
                idx = i;
            }
        }

        if (idx == -1) {
            gantt.push_back(-1);
            ++time;
        } else {
            --processes[idx].rt;
            gantt.push_back(processes[idx].pid);
            ++time;

            if (processes[idx].rt == 0) {
                processes[idx].ct = time;
                ++completed;
            }
        }
    }

    return finalize(std::move(processes), std::move(gantt));
}

ScheduleResult schedule_rr(std::vector<Process> processes, int time_quantum) {
    if (time_quantum <= 0) time_quantum = 1;

    std::queue<int> q;
    const int n = static_cast<int>(processes.size());
    int time = 0;
    int completed = 0;
    std::vector<int> gantt;

    std::vector<bool> in_q(n, false);

    for (auto& pr : processes) pr.rt = pr.bt;

    while (completed < n) {
        for (int i = 0; i < n; ++i) {
            if (processes[i].at <= time && !in_q[i] && processes[i].rt > 0) {
                q.push(i);
                in_q[i] = true;
            }
        }

        if (q.empty()) {
            gantt.push_back(-1);
            ++time;
            continue;
        }

        const int i = q.front();
        q.pop();

        const int run = (std::min)(time_quantum, processes[i].rt);
        for (int j = 0; j < run; ++j) {
            gantt.push_back(processes[i].pid);
            ++time;

            for (int k = 0; k < n; ++k) {
                if (processes[k].at <= time && !in_q[k] && processes[k].rt > 0) {
                    q.push(k);
                    in_q[k] = true;
                }
            }
        }

        processes[i].rt -= run;

        if (processes[i].rt > 0)
            q.push(i);
        else {
            processes[i].ct = time;
            ++completed;
        }
    }

    return finalize(std::move(processes), std::move(gantt));
}

ScheduleResult schedule_priority(std::vector<Process> processes) {
    const int n = static_cast<int>(processes.size());
    int time = 0;
    int completed = 0;
    std::vector<int> gantt;

    while (completed < n) {
        int idx = -1;
        int best = INT_MAX;

        for (int i = 0; i < n; ++i) {
            if (processes[i].at <= time && processes[i].ct == 0 && processes[i].priority < best) {
                best = processes[i].priority;
                idx = i;
            }
        }

        if (idx == -1) {
            gantt.push_back(-1);
            ++time;
        } else {
            for (int i = 0; i < processes[idx].bt; ++i) {
                gantt.push_back(processes[idx].pid);
                ++time;
            }
            processes[idx].ct = time;
            ++completed;
        }
    }

    return finalize(std::move(processes), std::move(gantt));
}

ScheduleResult run_scheduler(std::vector<Process> processes, Algorithm algo, int time_quantum) {
    switch (algo) {
        case Algorithm::FCFS:
            return schedule_fcfs(std::move(processes));
        case Algorithm::SJF:
            return schedule_sjf(std::move(processes));
        case Algorithm::SRTF:
            return schedule_srtf(std::move(processes));
        case Algorithm::RR:
            return schedule_rr(std::move(processes), time_quantum);
        case Algorithm::Priority:
            return schedule_priority(std::move(processes));
    }
    return {};
}

std::vector<Process> generate_random_processes(int n) {
    std::vector<Process> p(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        Process pr;
        pr.pid = i + 1;
        pr.at = std::rand() % 5;
        pr.bt = std::rand() % 10 + 1;
        pr.rt = 0;
        pr.ct = 0;
        pr.tat = 0;
        pr.wt = 0;
        pr.priority = std::rand() % 5;
        p[static_cast<size_t>(i)] = pr;
    }
    return p;
}
