#ifndef JUDGE_CLIENT_RUN_HELPERS_H
#define JUDGE_CLIENT_RUN_HELPERS_H

#include <memory>
#include <utility>

#include <sys/types.h>
#include <unistd.h>

#include "judge_client_context.h"
#include "judge_client_run_path_helpers.h"
#include "judge_client_run_series_helpers.h"
#include "header/static_var.h"

class Language;

namespace judge_run_helpers {

void configure_and_run(std::shared_ptr<Language> &languageModel, char *work_dir, const double &time_lmt,
                       const double &usedtime, const int &mem_lmt, double mem_cur_factor,
                       double mem_max_factor, const JudgeConfigSnapshot &config);

void run_solution_common(int &lang, char *work_dir, const double &time_lmt, const double &usedtime,
                         const int &mem_lmt, const char *inputFile, const char *userOutputFile,
                         const char *errorOutputFile, double mem_cur_factor, double mem_max_factor,
                         const JudgeConfigSnapshot &config);
void run_solution_common(int &lang, char *work_dir, const double &time_lmt, const double &usedtime,
                         const int &mem_lmt, const char *inputFile, const char *userOutputFile,
                         const char *errorOutputFile, double mem_cur_factor, double mem_max_factor,
                         const JudgeConfigSnapshot &config, const LanguageFactory &language_factory);

void run_solution_common(int &lang, const JudgePaths &paths, const double &time_lmt, const double &usedtime,
                         const int &mem_lmt, double mem_cur_factor, double mem_max_factor,
                         const JudgeConfigSnapshot &config);
void run_solution_common(int &lang, const JudgePaths &paths, const double &time_lmt, const double &usedtime,
                         const int &mem_lmt, double mem_cur_factor, double mem_max_factor,
                         const JudgeConfigSnapshot &config, const LanguageFactory &language_factory);

template <typename F>
pid_t fork_and_run_child(F fn) {
    pid_t pid = fork();
    if (pid == 0) {
        fn();
        exit(0);
    }
    return pid;
}

void prepare_run_files_with_id(int language, int runner_id, const std::pair<std::string, int> &infilePair,
                               int problemId, char *work_dir, int num_of_test,
                               int call_counter_local[call_array_size], char *infile, char *outfile,
                               char *userfile, const int *syscall_template, bool record_syscall);

JudgeResult finish_run_with_id(pid_t pid, int &ACflg, int SPECIAL_JUDGE, int solution_id, int language,
                               int &topmemory, int memoryLimit, double &usedtime, double timeLimit, int problemId,
                               int &PEflg, char *work_dir, int num_of_test,
                               int call_counter_local[call_array_size], char *infile, char *outfile, char *userfile,
                               char *usercode, std::string &global_work_dir, const JudgeConfigSnapshot &config,
                               const JudgeEnv &env, bool record_syscall, bool debug_enabled);
JudgeResult finish_run_with_id(pid_t pid, int &ACflg, int SPECIAL_JUDGE, int solution_id, int language,
                               int &topmemory, int memoryLimit, double &usedtime, double timeLimit, int problemId,
                               int &PEflg, char *work_dir, int num_of_test,
                               int call_counter_local[call_array_size], char *infile, char *outfile, char *userfile,
                               char *usercode, std::string &global_work_dir, const JudgeConfigSnapshot &config,
                               const JudgeEnv &env, bool record_syscall, bool debug_enabled,
                               const LanguageFactory &language_factory, const CompareFactory &compare_factory);

}  // namespace judge_run_helpers

#endif
