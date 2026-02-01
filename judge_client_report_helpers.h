#pragma once

#include <functional>
#include <string>

#include "header/static_var.h"

using ResultSender = std::function<void(const std::string &)>;

std::string build_test_run_output(int ACflg, double &usedtime, double timeLimit, int solution_id,
                                  const char *work_dir, bool debug_enabled);

void send_test_run_bundle(int solution_id, double usedtime, int topmemory, const std::string &test_run_out,
                          const ResultSender &sender);
