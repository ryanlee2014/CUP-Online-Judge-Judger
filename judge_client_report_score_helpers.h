#ifndef JUDGE_CLIENT_REPORT_SCORE_HELPERS_H
#define JUDGE_CLIENT_REPORT_SCORE_HELPERS_H

#include "judge_client_report_helpers.h"

double calculate_pass_rate(double pass_rate, int num_of_test);

void apply_parallel_result(const JudgeSeriesResult &r, int num_of_test, double timeLimit, int memoryLimit,
                           int &finalACflg, int &ACflg, int &topmemory, double &usedtime,
                           double &max_case_time, int &pass_point, double &pass_rate,
                           const ResultSender &sender);

#endif
