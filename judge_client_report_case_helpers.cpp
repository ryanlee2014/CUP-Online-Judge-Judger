#include "judge_client_report_case_helpers.h"

#include <iostream>

#include "header/static_var.h"
#include "judge_client_report_helpers.h"
#include "judge_client_run.h"
#include "judge_client_watch.h"
#include "library/judge_lib.h"
#include "manager/syscall/InitManager.h"

using namespace std;

extern int call_counter[call_array_size];

template <typename F>
static pid_t fork_and_run_child(F fn) {
    pid_t pid = fork();
    if (pid == CHILD_PROCESS) {
        fn();
        exit(0);
    }
    return pid;
}

void run_single_testcase(JudgeContext &ctx, int runner_id, int solution_id,
                         int num_of_test, int memoryLimit, double timeLimit, char *work_dir,
                         char *infile, char *outfile, char *userfile, char *usercode,
                         string &global_work_dir, int &topmemory, int &ACflg, int &PEflg,
                         int &pass_point, double &pass_rate, int &finalACflg, double &usedtime,
                         double &max_case_time, const int *syscall_template,
                         const pair<string, int> &infilePair) {
    prepare_files(infilePair.first.c_str(), infilePair.second, infile, ctx.p_id, work_dir, outfile,
                  userfile, runner_id);
    if (syscall_template) {
        memcpy(call_counter, syscall_template, sizeof(int) * call_array_size);
    } else {
        InitManager::initSyscallLimits(ctx.lang, call_counter, ctx.flags.record_call != 0, call_array_size);
    }
    pid_t pidApp = fork_and_run_child([&]() {
        run_solution(ctx.lang, work_dir, timeLimit, usedtime, memoryLimit, ctx.config, ctx.language_factory);
    });
    if (pidApp != CHILD_PROCESS) {
        watch_solution_ex(pidApp, infile, ACflg, ctx.special_judge, userfile, outfile,
                          solution_id, ctx.lang, topmemory, memoryLimit, usedtime, timeLimit,
                          ctx.p_id, PEflg, work_dir, ctx.config, ctx.env,
                          ctx.flags.record_call != 0, ctx.flags.debug != 0);
        judge_solution(ctx, ACflg, usedtime, timeLimit, ctx.special_judge, infile,
                       outfile, userfile, usercode, PEflg, work_dir, topmemory,
                       memoryLimit, solution_id, num_of_test, global_work_dir);
        if (ctx.config.use_max_time) {
            max_case_time = max(usedtime, max_case_time);
            usedtime = ZERO_TIME;
        }
    }

    if (usedtime > timeLimit * 1000 || ACflg == TIME_LIMIT_EXCEEDED) {
        cout << "Time Limit Exceeded" << endl;
        ACflg = TIME_LIMIT_EXCEEDED;
        usedtime = timeLimit * 1000;
    }

    if (ACflg == ACCEPT) {
        ++pass_point;
    }

    if (ctx.config.all_test_mode) {
        if (ACflg == ACCEPT) {
            ++pass_rate;
        }
        if (finalACflg < ACflg) {
            finalACflg = ACflg;
        }
        ACflg = ACCEPT;
    }
}
