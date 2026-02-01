#include "judge_client_report.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <string>

#include "header/static_var.h"
#include "judge_client_context.h"
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

double clamp_time_limit(double timeLimit) {
    if (timeLimit > 300 * SECOND || timeLimit < ZERO) {
        return 300 * SECOND;
    }
    return timeLimit;
}

int clamp_memory_limit(int memoryLimit) {
    if (memoryLimit > ONE_KILOBYTE || memoryLimit < ONE) {
        return ONE_KILOBYTE;
    }
    return memoryLimit;
}

void send_compiling_bundle(int solution_id, const ResultSender &sender) {
    bundle.clear();
    bundle.setJudger(http_username);
    bundle.setSolutionID(solution_id);
    bundle.setResult(COMPILING);
    bundle.setFinished(NOT_FINISHED);
    bundle.setUsedTime(ZERO_TIME);
    bundle.setMemoryUse(ZERO_MEMORY);
    bundle.setPassPoint(ZERO_PASSPOINT);
    bundle.setPassRate(ZERO_PASSRATE);
    sender(bundle.toJSONString());
}

static std::string join_path(const char *base, const char *name) {
    return (std::filesystem::path(base) / name).string();
}

static int resolve_final_result(int ac, int final_ac, int pe, const JudgeConfigSnapshot &config) {
    int result = config.all_test_mode ? final_ac : ac;
    if (result == ACCEPT && pe == PRESENTATION_ERROR) {
        return PRESENTATION_ERROR;
    }
    return result;
}

static void update_bundle_progress(double usedtime, int topmemory, double timeLimit, int memoryLimit,
                                   int pass_point, double pass_rate) {
    bundle.setUsedTime(min(usedtime, timeLimit * 1000));
    bundle.setMemoryUse(min(topmemory / ONE_KILOBYTE, memoryLimit * STD_MB / ONE_KILOBYTE));
    bundle.setPassPoint(pass_point);
    bundle.setPassRate(pass_rate);
}

void report_compile_error_and_exit(int solution_id, const string &judgerId, char *work_dir,
                                   const ResultSender &sender) {
    string compile_info = getFileContent(join_path(work_dir, "ce.txt").c_str());
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

void send_running_bundle(int solution_id, int total_point, bool include_total, const ResultSender &sender) {
    bundle.setJudger(http_username);
    bundle.setSolutionID(solution_id);
    bundle.setResult(RUNNING_JUDGING);
    if (include_total) {
        bundle.setTotalPoint(total_point);
    }
    sender(bundle.toJSONString());
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

void apply_parallel_result(const JudgeSeriesResult &r, int num_of_test, double timeLimit, int memoryLimit,
                           int &finalACflg, int &ACflg, int &topmemory, double &usedtime,
                           double &max_case_time, int &pass_point, double &pass_rate,
                           const ResultSender &sender) {
    update_bundle_progress(r.usedTime, r.topMemory, timeLimit, memoryLimit, r.pass_point,
                           double(r.pass_point) / max(num_of_test, 1));
    sender(bundle.toJSONString());
    finalACflg = ACflg = r.ACflg;
    topmemory = r.topMemory;
    usedtime = max_case_time = min(r.usedTime, timeLimit * 1000);
    pass_point = pass_rate = r.pass_point;
}

bool should_continue_cases(int ACflg, const JudgeConfigSnapshot &config) {
    if (!(config.all_test_mode || ACflg == ACCEPT || ACflg == PRESENTATION_ERROR) && ACflg != TIME_LIMIT_EXCEEDED) {
        return false;
    }
    if (ACflg == RUNTIME_ERROR) {
        return false;
    }
    return true;
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

double calculate_pass_rate(double pass_rate, int num_of_test) {
    if (num_of_test <= 0) {
        return ZERO_PASSRATE;
    }
    return pass_rate / num_of_test;
}

void send_progress_update(double usedtime, int topmemory, double timeLimit, int memoryLimit,
                          int pass_point, double pass_rate, const ResultSender &sender) {
    update_bundle_progress(usedtime, topmemory, timeLimit, memoryLimit, pass_point, pass_rate);
    sender(bundle.toJSONString());
}

void finalize_result_and_send(int &ACflg, int &finalACflg, int &PEflg, int solution_id, int lang,
                              int p_id, shared_ptr<Language> &languageModel, double timeLimit,
                              int &topmemory, int &pass_point, double &pass_rate, int num_of_test,
                              double &usedtime, double &max_case_time, int &sim, int &sim_s_id,
                              const JudgeConfigSnapshot &config, const char *work_dir,
                              const ResultSender &sender, bool debug_enabled) {
    int result = resolve_final_result(ACflg, finalACflg, PEflg, config);
    if (config.sim_enable && result == ACCEPT && (languageModel->enableSim())) {
        sim = get_sim(solution_id, lang, p_id, sim_s_id, work_dir);
    } else {
        sim = ZERO_SIM;
    }
    string runtimeInfo;
    if (result == RUNTIME_ERROR) {
        runtimeInfo = getRuntimeInfoContents(join_path(work_dir, "error.out").c_str());
        if (debug_enabled)
            printf("add RE info of %d..... \n", solution_id);
    }
    if (config.use_max_time) {
        usedtime = max_case_time;
    }
    if (result == TIME_LIMIT_EXCEEDED) {
        usedtime = timeLimit * 1000;
    }
    if (result == WRONG_ANSWER || result == PRESENTATION_ERROR) {
        runtimeInfo = getRuntimeInfoContents(join_path(work_dir, "diff.out").c_str());
        if (debug_enabled)
            printf("add diff info of %d..... \n", solution_id);
    }
    bundle.setResult(result);
    bundle.setFinished(FINISHED);
    bundle.setRuntimeInfo(runtimeInfo);
    bundle.setUsedTime(usedtime);
    bundle.setMemoryUse(topmemory / ONE_KILOBYTE);
    bundle.setPassPoint(pass_point);
    bundle.setPassRate(calculate_pass_rate(pass_rate, num_of_test));
    bundle.setSim(sim);
    bundle.setSimSource(sim_s_id);
    sender(bundle.toJSONString());
}

void finalize_submission(const string &judgerId, char *work_dir) {
    clean_workdir(work_dir);
    string judger = judgerId;
    removeSubmissionInfo(judger);
}

