#pragma once

#include <string>
#include <utility>

#include "judge_client_context.h"

struct CaseExecutionInput {
    JudgeContext &ctx;
    int runner_id = 0;
    int solution_id = 0;
    int num_of_test = 0;
    int memory_limit = 0;
    double time_limit = 0.0;
    char *work_dir = nullptr;
    char *infile = nullptr;
    char *outfile = nullptr;
    char *userfile = nullptr;
    char *usercode = nullptr;
    std::string *global_work_dir = nullptr;
    const int *syscall_template = nullptr;
    const std::pair<std::string, int> *infile_pair = nullptr;
    int ACflg = ACCEPT;
    int PEflg = ACCEPT;
    int topmemory = ZERO_MEMORY;
    int pass_point = ZERO_PASSPOINT;
    int finalACflg = ACCEPT;
    double pass_rate = ZERO_PASSRATE;
    double usedtime = ZERO_TIME;
    double max_case_time = ZERO_TIME;
};

struct CaseExecutionOutput {
    int ACflg = ACCEPT;
    int PEflg = ACCEPT;
    int topmemory = ZERO_MEMORY;
    int pass_point = ZERO_PASSPOINT;
    int finalACflg = ACCEPT;
    double pass_rate = ZERO_PASSRATE;
    double usedtime = ZERO_TIME;
    double max_case_time = ZERO_TIME;
};

CaseExecutionOutput execute_single_case(const CaseExecutionInput &input);
