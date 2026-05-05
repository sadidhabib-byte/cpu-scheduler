#pragma once

#include <string>
#include <vector>

struct Process {
    int pid{};
    int at{};
    int bt{};
    int rt{};
    int ct{};
    int tat{};
    int wt{};
    int priority{};
};

struct ScheduleResult {
    std::vector<Process> processes;
    std::vector<int> gantt;
    float avg_wt{};
    float avg_tat{};
};

enum class Algorithm { FCFS, SJF, SRTF, RR, Priority };

[[nodiscard]] ScheduleResult schedule_fcfs(std::vector<Process> processes);
[[nodiscard]] ScheduleResult schedule_sjf(std::vector<Process> processes);
[[nodiscard]] ScheduleResult schedule_srtf(std::vector<Process> processes);
[[nodiscard]] ScheduleResult schedule_rr(std::vector<Process> processes, int time_quantum);
[[nodiscard]] ScheduleResult schedule_priority(std::vector<Process> processes);

[[nodiscard]] ScheduleResult run_scheduler(std::vector<Process> processes, Algorithm algo,
                                           int time_quantum);

[[nodiscard]] std::vector<Process> generate_random_processes(int n);
