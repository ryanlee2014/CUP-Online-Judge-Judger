#pragma once

#include <functional>
#include <string>

#include "judge_client_context.h"

using ResultSender = std::function<void(const std::string &)>;

void send_test_run_bundle(int solution_id, double usedtime, int topmemory, const std::string &test_run_out,
                          const ResultSender &sender);

std::string join_report_path(const char *base, const char *name);

void update_bundle_progress(double usedtime, int topmemory, double timeLimit, int memoryLimit,
                            int pass_point, double pass_rate);

void adjust_usedtime_for_result(int result, double timeLimit, bool use_max_time,
                                double &usedtime, double max_case_time);

std::string build_runtime_info(int result, int solution_id, const char *work_dir, bool debug_enabled);
