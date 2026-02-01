#ifndef JUDGE_CLIENT_REPORT_FINALIZE_HELPERS_H
#define JUDGE_CLIENT_REPORT_FINALIZE_HELPERS_H

#include <string>

#include "judge_client_context.h"
#include "model/judge/language/Language.h"

int resolve_final_result(int ac, int final_ac, int pe, const JudgeConfigSnapshot &config);

void apply_runtime_info_and_time(int result, int solution_id, const JudgeConfigSnapshot &config,
                                 const char *work_dir, bool debug_enabled,
                                 double timeLimit, double max_case_time,
                                 double &usedtime, std::string &runtimeInfo);

#endif
