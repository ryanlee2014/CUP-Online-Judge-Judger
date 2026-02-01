#include "judge_client_report.h"

#include <cstdio>
#include <string>

#include "header/static_var.h"
#include "judge_client_report_bundle_helpers.h"
#include "judge_client_report_helpers.h"
#include "judge_client_report_test_run_helpers.h"
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

void report_compile_error_and_exit(int solution_id, const string &judgerId, char *work_dir,
                                   const ResultSender &sender) {
    string compile_info = getFileContent(join_report_path(work_dir, "ce.txt").c_str());
    bundle.setJudger(http_username);
    bundle.setSolutionID(solution_id);
    bundle.setResult(COMPILE_ERROR);
    bundle.setFinished(FINISHED);
    bundle.setCompileInfo(compile_info);
    sender(bundle.toJSONString());
    clean_workdir(work_dir);
    string judger = judgerId;
    removeSubmissionInfo(judger);
    write_log(oj_home, "compile error");
    umount(work_dir);
    exit(0);
}

void handle_test_run(int solution_id, int lang, int p_id, int SPECIAL_JUDGE, double timeLimit,
                     int memoryLimit, char *work_dir, char *infile, char *outfile, char *userfile,
                     int &topmemory, int &ACflg, int &PEflg, double &usedtime, const string &judgerId,
                     shared_ptr<ISubmissionAdapter> &adapter, SubmissionInfo &submissionInfo,
                     shared_ptr<Language> &languageModel, const JudgeConfigSnapshot &config,
                     const ResultSender &sender, bool record_syscall, bool debug_enabled) {
    printf("running a custom input...\n");
    if (MYSQL_MODE) {
        adapter->getCustomInput(solution_id, work_dir);
    } else {
        getCustomInputFromSubmissionInfo(submissionInfo, work_dir);
    }
    InitManager::initSyscallLimits(lang, call_counter, record_call, call_array_size);
    send_running_bundle(solution_id, 0, false, sender);
    pid_t pidApp = fork_and_run_child([&]() {
        run_solution(lang, work_dir, timeLimit, usedtime, memoryLimit, config);
    });
    if (pidApp != CHILD_PROCESS) {
        watch_solution_ex(pidApp, infile, ACflg, SPECIAL_JUDGE, userfile, outfile,
                          solution_id, lang, topmemory, memoryLimit, usedtime, timeLimit,
                          p_id, PEflg, work_dir, config, record_syscall, debug_enabled);
    }
    ACflg = languageModel->fixACStatus(ACflg);
    string test_run_out = build_test_run_output(ACflg, usedtime, timeLimit, solution_id, work_dir, debug_enabled);
    send_test_run_bundle(solution_id, usedtime, topmemory, test_run_out, sender);
    clean_workdir(work_dir);
    string judger = judgerId;
    removeSubmissionInfo(judger);
    exit(0);
}
