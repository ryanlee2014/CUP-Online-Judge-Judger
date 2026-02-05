#include "judge_client_run_helpers.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/ptrace.h>
#include <unistd.h>

#include "external/compare/Compare.h"
#include "judge_client_run_limit_helpers.h"
#include "judge_client_run_io_helpers.h"
#include "judge_client_watch.h"
#include "judge_client_run.h"
#include "library/judge_lib.h"
#include "manager/syscall/InitManager.h"
#include "model/judge/language/Language.h"
#include "model/judge/policy/SpecialJudge.h"

using namespace std;

namespace judge_run_helpers {

void configure_and_run(shared_ptr<Language> &languageModel, char *work_dir, const double &time_lmt,
                       const double &usedtime, const int &mem_lmt, double mem_cur_factor,
                       double mem_max_factor, const JudgeConfigSnapshot &config) {
    if (config.use_ptrace) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    }
    languageModel->buildChrootSandbox(work_dir);
    setRunUser();
    apply_cpu_time_limit(config, time_lmt, usedtime);
    apply_file_size_limit();
    languageModel->setProcessLimit();
    apply_stack_limit();
    apply_memory_limit(*languageModel, mem_lmt, mem_cur_factor, mem_max_factor);
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
    JudgePaths paths = build_case_paths(work_dir, inputFile, userOutputFile, errorOutputFile);
    run_solution_common(lang, paths, time_lmt, usedtime, mem_lmt, mem_cur_factor, mem_max_factor, config);
}

void run_solution_common(int &lang, char *work_dir, const double &time_lmt, const double &usedtime,
                         const int &mem_lmt, const char *inputFile, const char *userOutputFile,
                         const char *errorOutputFile, double mem_cur_factor, double mem_max_factor,
                         const JudgeConfigSnapshot &config, const LanguageFactory &language_factory) {
    JudgePaths paths = build_case_paths(work_dir, inputFile, userOutputFile, errorOutputFile);
    run_solution_common(lang, paths, time_lmt, usedtime, mem_lmt, mem_cur_factor, mem_max_factor, config,
                        language_factory);
}

void run_solution_common(int &lang, const JudgePaths &paths, const double &time_lmt, const double &usedtime,
                         const int &mem_lmt, double mem_cur_factor, double mem_max_factor,
                         const JudgeConfigSnapshot &config) {
    shared_ptr<Language> languageModel(getLanguageModel(lang));
    int nice_result = nice(19);
    (void) nice_result;
    set_child_work_dir(paths.work_dir.c_str());
    string input_path;
    string output_path;
    string error_path;
    prepare_io_paths(paths.work_dir.c_str(), paths.infile.c_str(), paths.userfile.c_str(),
                     paths.errorfile.c_str(), input_path, output_path, error_path);
    redirect_stdio(input_path, output_path, error_path);
    configure_and_run(languageModel, const_cast<char *>(paths.work_dir.c_str()), time_lmt, usedtime, mem_lmt,
                      mem_cur_factor, mem_max_factor, config);
}

void run_solution_common(int &lang, const JudgePaths &paths, const double &time_lmt, const double &usedtime,
                         const int &mem_lmt, double mem_cur_factor, double mem_max_factor,
                         const JudgeConfigSnapshot &config, const LanguageFactory &language_factory) {
    auto languageModel = language_factory ? language_factory(lang) : shared_ptr<Language>(getLanguageModel(lang));
    int nice_result = nice(19);
    (void) nice_result;
    set_child_work_dir(paths.work_dir.c_str());
    string input_path;
    string output_path;
    string error_path;
    prepare_io_paths(paths.work_dir.c_str(), paths.infile.c_str(), paths.userfile.c_str(),
                     paths.errorfile.c_str(), input_path, output_path, error_path);
    redirect_stdio(input_path, output_path, error_path);
    configure_and_run(languageModel, const_cast<char *>(paths.work_dir.c_str()), time_lmt, usedtime, mem_lmt,
                      mem_cur_factor, mem_max_factor, config);
}

void prepare_run_files_with_id(int language, int runner_id, const pair<string, int> &infilePair, int problemId,
                               char *work_dir, int num_of_test, int call_counter_local[call_array_size],
                               char *infile, char *outfile, char *userfile, const int *syscall_template,
                               bool record_syscall) {
    if (syscall_template) {
        memcpy(call_counter_local, syscall_template, sizeof(int) * call_array_size);
    } else {
        InitManager::initSyscallLimits(language, call_counter_local, record_syscall, call_array_size);
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
                               string &global_work_dir, const JudgeConfigSnapshot &config,
                               const JudgeEnv &env, bool record_syscall, bool debug_enabled) {
    return finish_run_with_id(pid, ACflg, SPECIAL_JUDGE, solution_id, language, topmemory, memoryLimit, usedtime,
                              timeLimit, problemId, PEflg, work_dir, num_of_test, call_counter_local, infile, outfile,
                              userfile, usercode, global_work_dir, config, env, record_syscall, debug_enabled,
                              nullptr, nullptr);
}

JudgeResult finish_run_with_id(pid_t pid, int &ACflg, int SPECIAL_JUDGE, int solution_id, int language,
                               int &topmemory, int memoryLimit, double &usedtime, double timeLimit, int problemId,
                               int &PEflg, char *work_dir, int num_of_test,
                               int call_counter_local[call_array_size],
                               char *infile, char *outfile, char *userfile, char *usercode,
                               string &global_work_dir, const JudgeConfigSnapshot &config,
                               const JudgeEnv &env, bool record_syscall, bool debug_enabled,
                               const LanguageFactory &language_factory, const CompareFactory &compare_factory) {
    watch_solution_with_file_id_ex(pid, infile, ACflg, SPECIAL_JUDGE, userfile, outfile,
                                   solution_id, language, topmemory, memoryLimit, usedtime, timeLimit,
                                   problemId, PEflg, work_dir, num_of_test, call_counter_local, config,
                                   env, record_syscall, debug_enabled);
    if (usedtime > timeLimit * 1000 || ACflg == TIME_LIMIT_EXCEEDED) {
        usedtime = timeLimit * 1000;
        ACflg = TIME_LIMIT_EXCEEDED;
    }
    JudgeContext ctx;
    ctx.lang = language;
    ctx.p_id = problemId;
    ctx.config = config;
    ctx.env = env;
    ctx.flags.debug = debug_enabled ? 1 : 0;
    ctx.flags.record_call = record_syscall ? 1 : 0;
    ctx.language_factory = language_factory;
    ctx.compare_factory = compare_factory;
    judge_solution(ctx, ACflg, usedtime, timeLimit, SPECIAL_JUDGE, infile,
                   outfile, userfile, usercode, PEflg, work_dir, topmemory,
                   memoryLimit, solution_id, num_of_test, global_work_dir);
    return {ACflg, usedtime, topmemory, num_of_test};
}

}  // namespace judge_run_helpers
