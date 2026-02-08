#pragma once

#include <functional>
#include <string>
#include <sys/types.h>
#include <utility>

#include "judge_client_context.h"

struct CaseExecutionLimits {
    int num_of_test = 0;
    int memory_limit = 0;
    double time_limit = 0.0;
};

struct CaseExecutionIO {
    JudgePaths *paths = nullptr;
    char *usercode = nullptr;
    std::string *global_work_dir = nullptr;
    const int *syscall_template = nullptr;
    const std::pair<std::string, int> *infile_pair = nullptr;
};

struct CaseExecutionState {
    int ACflg = ACCEPT;
    int PEflg = ACCEPT;
    int topmemory = ZERO_MEMORY;
    int pass_point = ZERO_PASSPOINT;
    int finalACflg = ACCEPT;
    double pass_rate = ZERO_PASSRATE;
    double usedtime = ZERO_TIME;
    double max_case_time = ZERO_TIME;
};

struct CaseExecutionInput {
    JudgeContext &ctx;
    int runner_id = 0;
    int solution_id = 0;
    CaseExecutionLimits limits;
    CaseExecutionIO io;
    CaseExecutionState state;
};

using CaseExecutionOutput = CaseExecutionState;

CaseExecutionOutput execute_single_case(const CaseExecutionInput &input);

using CaseSpawnFunction = std::function<pid_t(const std::function<void()> &)>;
void set_case_spawn_function_for_test(const CaseSpawnFunction &spawn_fn);
void reset_case_spawn_function_for_test();
