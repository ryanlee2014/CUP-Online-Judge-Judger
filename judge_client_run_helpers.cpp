#include "judge_client_run_helpers.h"

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <sys/ptrace.h>
#include <sys/resource.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "external/compare/Compare.h"
#include "judge_client_watch.h"
#include "judge_client_run.h"
#include "library/judge_lib.h"
#include "manager/syscall/InitManager.h"
#include "model/judge/language/Language.h"
#include "model/judge/policy/SpecialJudge.h"

using namespace std;

namespace judge_run_helpers {

std::string join_path(const char *base, const char *name) {
    return (std::filesystem::path(base) / name).string();
}

void set_child_work_dir(const char *work_dir) {
    if (chdir(work_dir) != 0 && DEBUG) {
        perror("chdir");
    }
}

int read_env_int(const char *name) {
    const char *val = std::getenv(name);
    if (!val || !*val) {
        return -1;
    }
    char *end = nullptr;
    long parsed = std::strtol(val, &end, 10);
    if (end == val || *end != '\0' || parsed <= 0 || parsed > INT_MAX) {
        return -1;
    }
    return static_cast<int>(parsed);
}

void configure_and_run(shared_ptr<Language> &languageModel, char *work_dir, const double &time_lmt,
                       const double &usedtime, const int &mem_lmt, double mem_cur_factor,
                       double mem_max_factor, const JudgeConfigSnapshot &config) {
    if (config.use_ptrace) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    }
    languageModel->buildChrootSandbox(work_dir);
    setRunUser();
    struct rlimit LIM{};
    if (config.all_test_mode)
        LIM.rlim_cur = static_cast<rlim_t>(time_lmt + 1);
    else
        LIM.rlim_cur = static_cast<rlim_t>((time_lmt - usedtime / 1000) + 1);
    LIM.rlim_max = LIM.rlim_cur;
    setrlimit(RLIMIT_CPU, &LIM);
    alarm(0);
    alarm(static_cast<unsigned int>(time_lmt * 10));

    LIM.rlim_max = ((STD_F_LIM << 2) + STD_MB);
    LIM.rlim_cur = (STD_F_LIM << 2);
    setrlimit(RLIMIT_FSIZE, &LIM);
    languageModel->setProcessLimit();

    LIM.rlim_cur = static_cast<rlim_t>(STD_MB << 7);
    LIM.rlim_max = static_cast<rlim_t>(STD_MB << 7);
    setrlimit(RLIMIT_STACK, &LIM);

    LIM.rlim_cur = static_cast<rlim_t>(STD_MB * mem_lmt * mem_cur_factor);
    LIM.rlim_max = static_cast<rlim_t>(STD_MB * mem_lmt * mem_max_factor);
    languageModel->runMemoryLimit(LIM);
    if (!config.use_ptrace) {
        languageModel->buildSeccompSandbox();
    }
    languageModel->run(mem_lmt);
    fflush(stderr);
    exit(0);
}

void run_solution_common(int &lang, char *work_dir, const double &time_lmt, const double &usedtime,
                         const int &mem_lmt, const char *inputFile, const char *userOutputFile,
                         const char *errorOutputFile, double mem_cur_factor, double mem_max_factor,
                         const JudgeConfigSnapshot &config) {
    shared_ptr<Language> languageModel(getLanguageModel(lang));
    nice(19);
    set_child_work_dir(work_dir);
    string input_path = join_path(work_dir, inputFile);
    string output_path = join_path(work_dir, userOutputFile);
    string error_path = join_path(work_dir, errorOutputFile);
    freopen(input_path.c_str(), "r", stdin);
    freopen(output_path.c_str(), "w", stdout);
    freopen(error_path.c_str(), "a+", stderr);
    configure_and_run(languageModel, work_dir, time_lmt, usedtime, mem_lmt, mem_cur_factor, mem_max_factor, config);
}

void build_parallel_io_names(int file_id, char *input, char *userOutput, char *errorOutput) {
    snprintf(input, BUFFER_SIZE, "data%d.in", file_id);
    snprintf(userOutput, BUFFER_SIZE, "user%d.out", file_id);
    snprintf(errorOutput, BUFFER_SIZE, "error%d.out", file_id);
}

void prepare_run_files_with_id(int language, int runner_id, const pair<string, int> &infilePair, int problemId,
                               char *work_dir, int num_of_test, int call_counter_local[call_array_size],
                               char *infile, char *outfile, char *userfile, const int *syscall_template) {
    if (syscall_template) {
        memcpy(call_counter_local, syscall_template, sizeof(int) * call_array_size);
    } else {
        InitManager::initSyscallLimits(language, call_counter_local, record_call, call_array_size);
    }
    prepare_files_with_id(infilePair.first.c_str(), infilePair.second, infile, problemId, work_dir, outfile,
                          userfile, runner_id, num_of_test);
    cout << "infile: " << infile << " outfile " << outfile << " userfile " << userfile << endl;
    cout << "num of test: " << num_of_test << endl;
}

JudgeResult finish_run_with_id(pid_t pid, int &ACflg, int SPECIAL_JUDGE, int solution_id, int language,
                               int &topmemory, int memoryLimit, double &usedtime, double timeLimit, int problemId,
                               int &PEflg, char *work_dir, int num_of_test,
                               int call_counter_local[call_array_size],
                               char *infile, char *outfile, char *userfile, char *usercode,
                               string &global_work_dir, const JudgeConfigSnapshot &config) {
    watch_solution_with_file_id(pid, infile, ACflg, SPECIAL_JUDGE, userfile, outfile,
                                solution_id, language, topmemory, memoryLimit, usedtime, timeLimit,
                                problemId, PEflg, work_dir, num_of_test, call_counter_local, config);
    if (usedtime > timeLimit * 1000 || ACflg == TIME_LIMIT_EXCEEDED) {
        usedtime = timeLimit * 1000;
        ACflg = TIME_LIMIT_EXCEEDED;
    }
    judge_solution(ACflg, usedtime, timeLimit, SPECIAL_JUDGE, problemId, infile,
                   outfile, userfile, usercode, PEflg, language, work_dir, topmemory,
                   memoryLimit, solution_id, num_of_test, global_work_dir, config);
    return {ACflg, usedtime, topmemory, num_of_test};
}

void update_series_result(JudgeSeriesResult &finalResult, const JudgeResult &r) {
    cout << "Flag " << r.ACflg << "Memory " << r.topMemory << "UsedTime " << r.usedTime << "Num " << r.num << endl;
    finalResult.ACflg = max(finalResult.ACflg, r.ACflg);
    finalResult.topMemory = max(finalResult.topMemory, r.topMemory);
    finalResult.usedTime = max(finalResult.usedTime, r.usedTime);
    finalResult.pass_point += r.ACflg == ACCEPT;
}

}  // namespace judge_run_helpers
