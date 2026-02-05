#pragma once

#include <string>
#include <utility>
#include <vector>

#include "header/static_var.h"
#include "judge_client_context.h"
#include "model/submission/SubmissionInfo.h"

struct JudgeConfigSnapshot;

struct RunTaskOptions {
    int runner_id = 0;
    int language = 0;
    char *work_dir = nullptr;
    const std::pair<std::string, int> *infile_pair = nullptr;
    int ACflg = ACCEPT;
    int special_judge = NONE_SPECIAL_JUDGE;
    int solution_id = 0;
    double time_limit = 0;
    double used_time = 0;
    int memory_limit = 0;
    int problem_id = 0;
    char *usercode = nullptr;
    int case_index = 0;
    std::string *global_work_dir = nullptr;
    const JudgeConfigSnapshot *config = nullptr;
    const JudgeEnv *env = nullptr;
    bool record_syscall = false;
    bool debug_enabled = false;
    const int *syscall_template = nullptr;
    LanguageFactory language_factory;
    CompareFactory compare_factory;
};

struct ParallelRunOptions {
    int runner_id = 0;
    int language = 0;
    char *work_dir = nullptr;
    char *usercode = nullptr;
    int time_limit = 0;
    int used_time = 0;
    int memory_limit = 0;
    std::vector<std::pair<std::string, int>> *in_file_list = nullptr;
    int *ac_flag = nullptr;
    int special_judge = NONE_SPECIAL_JUDGE;
    std::string *global_work_dir = nullptr;
    SubmissionInfo *submission = nullptr;
    const JudgeConfigSnapshot *config = nullptr;
    const JudgeEnv *env = nullptr;
    bool record_syscall = false;
    bool debug_enabled = false;
    const int *syscall_template = nullptr;
    LanguageFactory language_factory;
    CompareFactory compare_factory;
};

int compute_parallel_budget(const ParallelRunOptions &opts);

void run_solution(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                  int &mem_lmt, const JudgeConfigSnapshot &config);
void run_solution(int &lang, char *work_dir, const double &time_lmt, const double &usedtime,
                  const int &mem_lmt, const JudgeConfigSnapshot &config);
void run_solution_parallel(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                           int &mem_lmt, int fileId, const JudgeConfigSnapshot &config);
void run_solution(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                  int &mem_lmt, const JudgeConfigSnapshot &config, const LanguageFactory &language_factory);
void run_solution(int &lang, char *work_dir, const double &time_lmt, const double &usedtime,
                  const int &mem_lmt, const JudgeConfigSnapshot &config, const LanguageFactory &language_factory);
void run_solution_parallel(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                           int &mem_lmt, int fileId, const JudgeConfigSnapshot &config,
                           const LanguageFactory &language_factory);

JudgeResult judge_solution(int &ACflg, double &usedtime, double time_lmt, int isspj,
                           int p_id, char *infile, char *outfile, char *userfile, char *usercode, int &PEflg,
                           int lang, char *work_dir, int &topmemory, int mem_lmt,
                           int solution_id, int num_of_test, std::string &global_work_dir,
                           const JudgeConfigSnapshot &config, const JudgeEnv &env, bool debug_enabled);
JudgeResult judge_solution(JudgeContext &ctx, int &ACflg, double &usedtime, double time_lmt, int isspj,
                           char *infile, char *outfile, char *userfile, char *usercode, int &PEflg,
                           char *work_dir, int &topmemory, int mem_lmt,
                           int solution_id, int num_of_test, std::string &global_work_dir);

JudgeResult runJudgeTask(const RunTaskOptions &opts);

JudgeResult runJudgeTask(int runner_id, int language, char *work_dir,
                         const std::pair<std::string, int> &infilePair,
                         int ACflg, int SPECIAL_JUDGE, int solution_id, double timeLimit, double usedtime,
                         int memoryLimit, int problemId, char *usercode, int num_of_test,
                         std::string &global_work_dir, const JudgeConfigSnapshot &config,
                         const JudgeEnv &env, bool record_syscall, bool debug_enabled,
                         const int *syscall_template);
JudgeResult runJudgeTask(int runner_id, int language, char *work_dir,
                         const std::pair<std::string, int> &infilePair,
                         int ACflg, int SPECIAL_JUDGE, int solution_id, double timeLimit, double usedtime,
                         int memoryLimit, int problemId, char *usercode, int num_of_test,
                         std::string &global_work_dir, const JudgeConfigSnapshot &config,
                         const JudgeEnv &env, bool record_syscall, bool debug_enabled,
                         const int *syscall_template, const LanguageFactory &language_factory);
JudgeResult runJudgeTask(int runner_id, int language, char *work_dir,
                         const std::pair<std::string, int> &infilePair,
                         int ACflg, int SPECIAL_JUDGE, int solution_id, double timeLimit, double usedtime,
                         int memoryLimit, int problemId, char *usercode, int num_of_test,
                         std::string &global_work_dir, const JudgeConfigSnapshot &config,
                         const JudgeEnv &env, bool record_syscall, bool debug_enabled,
                         const int *syscall_template, const LanguageFactory &language_factory,
                         const CompareFactory &compare_factory);
JudgeResult runJudgeTask(int runner_id, int language, char *work_dir,
                         const std::pair<std::string, int> &infilePair,
                         int ACflg, int SPECIAL_JUDGE, int solution_id, double timeLimit, double usedtime,
                         int memoryLimit, int problemId, char *usercode, int num_of_test,
                         std::string &global_work_dir, const JudgeConfigSnapshot &config,
                         const JudgeEnv &env, bool record_syscall, bool debug_enabled);

JudgeSeriesResult runParallelJudge(const ParallelRunOptions &opts);

JudgeSeriesResult runParallelJudge(int runner_id, int language, char *work_dir, char *usercode, int timeLimit,
                                   int usedtime, int memoryLimit,
                                   std::vector<std::pair<std::string, int>> &inFileList,
                                   int &ACflg, int SPECIAL_JUDGE, std::string &global_work_dir,
                                   SubmissionInfo &submissionInfo, const JudgeConfigSnapshot &config,
                                   const JudgeEnv &env, bool record_syscall, bool debug_enabled,
                                   const int *syscall_template);
JudgeSeriesResult runParallelJudge(int runner_id, int language, char *work_dir, char *usercode, int timeLimit,
                                   int usedtime, int memoryLimit,
                                   std::vector<std::pair<std::string, int>> &inFileList,
                                   int &ACflg, int SPECIAL_JUDGE, std::string &global_work_dir,
                                   SubmissionInfo &submissionInfo, const JudgeConfigSnapshot &config,
                                   const JudgeEnv &env, bool record_syscall, bool debug_enabled,
                                   const int *syscall_template, const LanguageFactory &language_factory);
JudgeSeriesResult runParallelJudge(int runner_id, int language, char *work_dir, char *usercode, int timeLimit,
                                   int usedtime, int memoryLimit,
                                   std::vector<std::pair<std::string, int>> &inFileList,
                                   int &ACflg, int SPECIAL_JUDGE, std::string &global_work_dir,
                                   SubmissionInfo &submissionInfo, const JudgeConfigSnapshot &config,
                                   const JudgeEnv &env, bool record_syscall, bool debug_enabled,
                                   const int *syscall_template, const LanguageFactory &language_factory,
                                   const CompareFactory &compare_factory);
JudgeSeriesResult runParallelJudge(int runner_id, int language, char *work_dir, char *usercode, int timeLimit,
                                   int usedtime, int memoryLimit,
                                   std::vector<std::pair<std::string, int>> &inFileList,
                                   int &ACflg, int SPECIAL_JUDGE, std::string &global_work_dir,
                                   SubmissionInfo &submissionInfo, const JudgeConfigSnapshot &config,
                                   const JudgeEnv &env, bool record_syscall, bool debug_enabled);
