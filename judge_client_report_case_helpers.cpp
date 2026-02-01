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

void run_single_testcase(int lang, int runner_id, int solution_id, int p_id, int SPECIAL_JUDGE,
                         int num_of_test, int memoryLimit, double timeLimit, char *work_dir,
                         char *infile, char *outfile, char *userfile, char *usercode,
                         string &global_work_dir, int &topmemory, int &ACflg, int &PEflg,
                         int &pass_point, double &pass_rate, int &finalACflg, double &usedtime,
                         double &max_case_time, const int *syscall_template,
                         const pair<string, int> &infilePair, const JudgeConfigSnapshot &config,
                         bool record_syscall, bool debug_enabled) {
    prepare_files(infilePair.first.c_str(), infilePair.second, infile, p_id, work_dir, outfile,
                  userfile, runner_id);
    if (syscall_template) {
        memcpy(call_counter, syscall_template, sizeof(int) * call_array_size);
    } else {
        InitManager::initSyscallLimits(lang, call_counter, record_call, call_array_size);
    }
    pid_t pidApp = fork_and_run_child([&]() {
        run_solution(lang, work_dir, timeLimit, usedtime, memoryLimit, config);
    });
    if (pidApp != CHILD_PROCESS) {
        watch_solution_ex(pidApp, infile, ACflg, SPECIAL_JUDGE, userfile, outfile,
                          solution_id, lang, topmemory, memoryLimit, usedtime, timeLimit,
                          p_id, PEflg, work_dir, config, record_syscall, debug_enabled);
        judge_solution(ACflg, usedtime, timeLimit, SPECIAL_JUDGE, p_id, infile,
                       outfile, userfile, usercode, PEflg, lang, work_dir, topmemory,
                       memoryLimit, solution_id, num_of_test, global_work_dir, config);
        if (config.use_max_time) {
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

    if (config.all_test_mode) {
        if (ACflg == ACCEPT) {
            ++pass_rate;
        }
        if (finalACflg < ACflg) {
            finalACflg = ACflg;
        }
        ACflg = ACCEPT;
    }
}
