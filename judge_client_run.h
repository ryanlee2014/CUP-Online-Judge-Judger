#pragma once

#include <string>
#include <utility>
#include <vector>

#include "header/static_var.h"
#include "judge_client_context.h"
#include "model/submission/SubmissionInfo.h"

struct JudgeConfigSnapshot;

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
                         const JudgeEnv &env, bool record_syscall, bool debug_enabled);

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
                                   const JudgeEnv &env, bool record_syscall, bool debug_enabled);
