#ifndef JUDGE_CLIENT_REPORT_TEST_RUN_HELPERS_H
#define JUDGE_CLIENT_REPORT_TEST_RUN_HELPERS_H

#include <string>

std::string build_test_run_output(int ACflg, double &usedtime, double timeLimit, int solution_id,
                                  const char *work_dir, bool debug_enabled);

#endif
