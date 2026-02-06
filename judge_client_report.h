#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "header/static_var.h"
#include "judge_client_adapter.h"
#include "judge_client_case_executor.h"
#include "judge_client_context.h"
#include "model/judge/language/Language.h"
#include "model/submission/SubmissionInfo.h"

struct JudgeConfigSnapshot;
using ResultSender = std::function<void(const std::string &)>;

double clamp_time_limit(double timeLimit);
int clamp_memory_limit(int memoryLimit);

void send_compiling_bundle(int solution_id, const JudgeEnv &env, const ResultSender &sender);
void report_compile_error_and_exit(int solution_id, const std::string &judgerId, char *work_dir,
                                   const JudgeEnv &env, const ResultSender &sender);
void send_running_bundle(int solution_id, int total_point, bool include_total,
                         const JudgeEnv &env, const ResultSender &sender);

void handle_test_run(int solution_id, int lang, int p_id, int SPECIAL_JUDGE, double timeLimit,
                     int memoryLimit, char *work_dir, char *infile, char *outfile, char *userfile,
                     int &topmemory, int &ACflg, int &PEflg, double &usedtime, const std::string &judgerId,
                     std::shared_ptr<ISubmissionAdapter> &adapter, SubmissionInfo &submissionInfo,
                     std::shared_ptr<Language> &languageModel, const JudgeConfigSnapshot &config,
                     const JudgeEnv &env, const ResultSender &sender,
                     bool mysql_mode, bool record_syscall, bool debug_enabled);

void apply_parallel_result(const JudgeSeriesResult &r, int num_of_test, double timeLimit, int memoryLimit,
                           int &finalACflg, int &ACflg, int &topmemory, double &usedtime,
                           double &max_case_time, int &pass_point, double &pass_rate,
                           const ResultSender &sender);
bool should_continue_cases(int ACflg, const JudgeConfigSnapshot &config);

struct RunSingleTestcaseRequest;
void run_single_testcase(JudgeContext &ctx, const RunSingleTestcaseRequest &request,
                         JudgePaths &paths, std::string &global_work_dir, CaseExecutionState &state);

double calculate_pass_rate(double pass_rate, int num_of_test);
void send_progress_update(double usedtime, int topmemory, double timeLimit, int memoryLimit,
                          int pass_point, double pass_rate, const ResultSender &sender);
void finalize_result_and_send(int &ACflg, int &finalACflg, int &PEflg, int solution_id, int lang,
                              int p_id, std::shared_ptr<Language> &languageModel, double timeLimit,
                              int &topmemory, int &pass_point, double &pass_rate, int num_of_test,
                              double &usedtime, double &max_case_time, int &sim, int &sim_s_id,
                              const JudgeConfigSnapshot &config, const char *work_dir,
                              const ResultSender &sender, bool debug_enabled);
void finalize_submission(const std::string &judgerId, char *work_dir);
