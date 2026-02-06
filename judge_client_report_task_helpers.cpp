#include "judge_client_report.h"

#include <cstdio>
#include <string>

#include "judge_client_process_utils.h"
#include "judge_client_report_bundle_helpers.h"
#include "judge_client_report_helpers.h"
#include "judge_client_report_test_run_helpers.h"
#include "judge_client_run.h"
#include "judge_client_watch.h"
#include "library/judge_lib.h"
#include "manager/syscall/InitManager.h"

using namespace std;

extern int call_counter[call_array_size];

static void prepare_test_run_input(int solution_id, int lang, char *work_dir,
                                   shared_ptr<ISubmissionAdapter> &adapter, SubmissionInfo &submissionInfo,
                                   const JudgeEnv &env, const ResultSender &sender,
                                   bool mysql_mode, bool record_syscall) {
    printf("running a custom input...\n");
    if (mysql_mode) {
        adapter->getCustomInput(solution_id, work_dir);
    } else {
        getCustomInputFromSubmissionInfo(submissionInfo, work_dir);
    }
    InitManager::initSyscallLimits(lang, call_counter, record_syscall, call_array_size);
    send_running_bundle(solution_id, 0, false, env, sender);
}

static void execute_test_run(int solution_id, int lang, int p_id, int SPECIAL_JUDGE, double timeLimit,
                             int memoryLimit, char *work_dir, char *infile, char *outfile, char *userfile,
                             int &topmemory, int &ACflg, int &PEflg, double &usedtime,
                             const string &judgerId, shared_ptr<Language> &languageModel,
                             const JudgeConfigSnapshot &config, const JudgeEnv &env,
                             const ResultSender &sender, bool record_syscall, bool debug_enabled) {
    pid_t pidApp = spawn_child([&]() {
        run_solution(lang, work_dir, timeLimit, usedtime, memoryLimit, config);
    });
    if (pidApp != CHILD_PROCESS) {
        watch_solution_ex(pidApp, infile, ACflg, SPECIAL_JUDGE, userfile, outfile,
                          solution_id, lang, topmemory, memoryLimit, usedtime, timeLimit,
                          p_id, PEflg, work_dir, config, env, record_syscall, debug_enabled);
    }
    ACflg = languageModel->fixACStatus(ACflg);
    string test_run_out = build_test_run_output(ACflg, usedtime, timeLimit, solution_id, work_dir, debug_enabled);
    send_test_run_bundle(solution_id, usedtime, topmemory, test_run_out, sender);
    clean_workdir(work_dir);
    string judger = judgerId;
    removeSubmissionInfo(judger);
    exit(0);
}

void report_compile_error_and_exit(int solution_id, const string &judgerId, char *work_dir,
                                   const JudgeEnv &env, const ResultSender &sender) {
    string compile_info = getFileContent(join_report_path(work_dir, "ce.txt").c_str());
    string judger_name = env.http_username;
    bundle.setJudger(judger_name);
    bundle.setSolutionID(solution_id);
    bundle.setResult(COMPILE_ERROR);
    bundle.setFinished(FINISHED);
    bundle.setCompileInfo(compile_info);
    sender(bundle.toJSONString());
    clean_workdir(work_dir);
    string judger = judgerId;
    removeSubmissionInfo(judger);
    write_log(env.oj_home.c_str(), "compile error");
    umount(work_dir);
    exit(0);
}

void handle_test_run(int solution_id, int lang, int p_id, int SPECIAL_JUDGE, double timeLimit,
                     int memoryLimit, char *work_dir, char *infile, char *outfile, char *userfile,
                     int &topmemory, int &ACflg, int &PEflg, double &usedtime, const string &judgerId,
                     shared_ptr<ISubmissionAdapter> &adapter, SubmissionInfo &submissionInfo,
                     shared_ptr<Language> &languageModel, const JudgeConfigSnapshot &config,
                     const JudgeEnv &env, const ResultSender &sender,
                     bool mysql_mode, bool record_syscall, bool debug_enabled) {
    prepare_test_run_input(solution_id, lang, work_dir, adapter, submissionInfo,
                           env, sender, mysql_mode, record_syscall);
    execute_test_run(solution_id, lang, p_id, SPECIAL_JUDGE, timeLimit, memoryLimit, work_dir,
                     infile, outfile, userfile, topmemory, ACflg, PEflg, usedtime, judgerId,
                     languageModel, config, env, sender, record_syscall, debug_enabled);
}
