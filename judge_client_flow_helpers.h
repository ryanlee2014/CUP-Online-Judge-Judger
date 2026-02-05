#pragma once

#include <string>
#include <vector>

#include "header/static_var.h"
#include "judge_client_context.h"

struct FlowState {
    char work_dir[BUFFER_SIZE]{};
    std::string global_work_dir;
    char fullpath[BUFFER_SIZE]{};
    char infile[BUFFER_SIZE]{};
    char outfile[BUFFER_SIZE]{};
    char userfile[BUFFER_SIZE]{};
    int ACflg = ACCEPT;
    int PEflg = ACCEPT;
    int topmemory = ZERO_MEMORY;
    double usedtime = ZERO_TIME;
    double max_case_time = ZERO_TIME;
    int pass_point = ZERO_PASSPOINT;
    double pass_rate = ZERO_PASSRATE;
    int finalACflg = ACCEPT;
    int sim = ZERO_SIM;
    int sim_s_id = ZERO_SIM;
    int num_of_test = 0;
};

std::string build_run_dir(int runner_id, const JudgeEnv &env);
void prepare_work_dir(const JudgeConfigSnapshot &config, char *work_dir);
void prepare_environment(JudgeContext &ctx, int runner_id, FlowState &state);
