#include "judge_client_case_executor.h"

#include <algorithm>
#include <cstring>
#include <iostream>

#include "judge_client_process_utils.h"
#include "judge_client_run.h"
#include "judge_client_watch.h"
#include "library/judge_lib.h"
#include "manager/syscall/InitManager.h"

using namespace std;

namespace {
CaseSpawnFunction &case_spawn_function() {
    static CaseSpawnFunction spawn_fn = [](const std::function<void()> &fn) {
        return spawn_child(fn);
    };
    return spawn_fn;
}

pid_t spawn_case_process(const std::function<void()> &fn) {
    return case_spawn_function()(fn);
}

void prepare_case_runtime_files(const CaseExecutionInput &input, char *infile, char *outfile, char *userfile) {
    char *work_dir = const_cast<char *>(input.io.paths->work_dir.c_str());
    prepare_files(input.io.infile_pair->first.c_str(), input.io.infile_pair->second, infile, input.ctx.p_id, work_dir,
                  outfile, userfile, input.runner_id);
    input.io.paths->infile = infile;
    input.io.paths->outfile = outfile;
    input.io.paths->userfile = userfile;
}

void init_case_call_counter(const CaseExecutionInput &input, int *call_counter_local) {
    if (input.io.syscall_template) {
        memcpy(call_counter_local, input.io.syscall_template, sizeof(int) * call_array_size);
        return;
    }
    InitManager::initSyscallLimits(input.ctx.lang, call_counter_local, input.ctx.flags.record_call != 0, call_array_size);
}

void apply_case_time_limit(CaseExecutionOutput &output, double time_limit_seconds) {
    if (output.usedtime <= time_limit_seconds * 1000 && output.ACflg != TIME_LIMIT_EXCEEDED) {
        return;
    }
    cout << "Time Limit Exceeded" << endl;
    output.ACflg = TIME_LIMIT_EXCEEDED;
    output.usedtime = time_limit_seconds * 1000;
}

void apply_case_result_policy(CaseExecutionOutput &output, const JudgeConfigSnapshot &config) {
    if (output.ACflg == ACCEPT) {
        ++output.pass_point;
    }
    if (!config.all_test_mode) {
        return;
    }
    if (output.ACflg == ACCEPT) {
        ++output.pass_rate;
    }
    if (output.finalACflg < output.ACflg) {
        output.finalACflg = output.ACflg;
    }
    output.ACflg = ACCEPT;
}
}  // namespace

void set_case_spawn_function_for_test(const CaseSpawnFunction &spawn_fn) {
    case_spawn_function() = spawn_fn;
}

void reset_case_spawn_function_for_test() {
    case_spawn_function() = [](const std::function<void()> &fn) {
        return spawn_child(fn);
    };
}

CaseExecutionOutput execute_single_case(const CaseExecutionInput &input) {
    CaseExecutionOutput output = input.state;
    char infile[BUFFER_SIZE] = {};
    char outfile[BUFFER_SIZE] = {};
    char userfile[BUFFER_SIZE] = {};
    int call_counter_local[call_array_size] = {};
    char *work_dir = const_cast<char *>(input.io.paths->work_dir.c_str());

    prepare_case_runtime_files(input, infile, outfile, userfile);
    init_case_call_counter(input, call_counter_local);
    pid_t pidApp = spawn_case_process([&]() {
        run_solution(input.ctx.lang, work_dir, input.limits.time_limit, output.usedtime, input.limits.memory_limit,
                     input.ctx.config, input.ctx.language_factory);
    });
    if (pidApp != CHILD_PROCESS) {
        watch_solution_ex(pidApp, infile, output.ACflg, input.ctx.special_judge, userfile, outfile, input.solution_id,
                          input.ctx.lang, output.topmemory, input.limits.memory_limit, output.usedtime,
                          input.limits.time_limit, input.ctx.p_id, output.PEflg,
                          work_dir, input.ctx.config, input.ctx.env, input.ctx.flags.record_call != 0,
                          input.ctx.flags.debug != 0, call_counter_local);
        judge_solution(input.ctx, output.ACflg, output.usedtime, input.limits.time_limit, input.ctx.special_judge,
                       infile, outfile, userfile, input.io.usercode, output.PEflg,
                       work_dir, output.topmemory, input.limits.memory_limit, input.solution_id, input.limits.num_of_test,
                       *input.io.global_work_dir);
        if (input.ctx.config.use_max_time) {
            output.max_case_time = max(output.usedtime, output.max_case_time);
            output.usedtime = ZERO_TIME;
        }
    }

    apply_case_time_limit(output, input.limits.time_limit);
    apply_case_result_policy(output, input.ctx.config);

    return output;
}
