#ifndef JUDGE_CLIENT_REPORT_BUNDLE_HELPERS_H
#define JUDGE_CLIENT_REPORT_BUNDLE_HELPERS_H

#include "judge_client_report_helpers.h"

void send_compiling_bundle(int solution_id, const JudgeEnv &env, const ResultSender &sender);
void send_running_bundle(int solution_id, int total_point, bool include_total,
                         const JudgeEnv &env, const ResultSender &sender);
void send_progress_update(double usedtime, int topmemory, double timeLimit, int memoryLimit,
                          int pass_point, double pass_rate, const ResultSender &sender);

#endif
