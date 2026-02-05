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

extern int call_counter[call_array_size];

CaseExecutionOutput execute_single_case(const CaseExecutionInput &input) {
    CaseExecutionOutput output;
    output.ACflg = input.ACflg;
    output.PEflg = input.PEflg;
    output.topmemory = input.topmemory;
    output.pass_point = input.pass_point;
    output.finalACflg = input.finalACflg;
    output.pass_rate = input.pass_rate;
    output.usedtime = input.usedtime;
    output.max_case_time = input.max_case_time;

    prepare_files(input.infile_pair->first.c_str(), input.infile_pair->second, input.infile, input.ctx.p_id,
                  input.work_dir, input.outfile, input.userfile, input.runner_id);
    if (input.syscall_template) {
        memcpy(call_counter, input.syscall_template, sizeof(int) * call_array_size);
    } else {
        InitManager::initSyscallLimits(input.ctx.lang, call_counter, input.ctx.flags.record_call != 0, call_array_size);
    }
    pid_t pidApp = spawn_child([&]() {
        run_solution(input.ctx.lang, input.work_dir, input.time_limit, output.usedtime, input.memory_limit,
                     input.ctx.config, input.ctx.language_factory);
    });
    if (pidApp != CHILD_PROCESS) {
        watch_solution_ex(pidApp, input.infile, output.ACflg, input.ctx.special_judge, input.userfile, input.outfile,
                          input.solution_id, input.ctx.lang, output.topmemory, input.memory_limit, output.usedtime,
                          input.time_limit, input.ctx.p_id, output.PEflg, input.work_dir, input.ctx.config, input.ctx.env,
                          input.ctx.flags.record_call != 0, input.ctx.flags.debug != 0);
        judge_solution(input.ctx, output.ACflg, output.usedtime, input.time_limit, input.ctx.special_judge, input.infile,
                       input.outfile, input.userfile, input.usercode, output.PEflg, input.work_dir, output.topmemory,
                       input.memory_limit, input.solution_id, input.num_of_test, *input.global_work_dir);
        if (input.ctx.config.use_max_time) {
            output.max_case_time = max(output.usedtime, output.max_case_time);
            output.usedtime = ZERO_TIME;
        }
    }

    if (output.usedtime > input.time_limit * 1000 || output.ACflg == TIME_LIMIT_EXCEEDED) {
        cout << "Time Limit Exceeded" << endl;
        output.ACflg = TIME_LIMIT_EXCEEDED;
        output.usedtime = input.time_limit * 1000;
    }

    if (output.ACflg == ACCEPT) {
        ++output.pass_point;
    }

    if (input.ctx.config.all_test_mode) {
        if (output.ACflg == ACCEPT) {
            ++output.pass_rate;
        }
        if (output.finalACflg < output.ACflg) {
            output.finalACflg = output.ACflg;
        }
        output.ACflg = ACCEPT;
    }

    return output;
}
