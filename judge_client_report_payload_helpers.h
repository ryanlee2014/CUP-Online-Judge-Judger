#ifndef JUDGE_CLIENT_REPORT_PAYLOAD_HELPERS_H
#define JUDGE_CLIENT_REPORT_PAYLOAD_HELPERS_H

#include <string>

#include "judge_client_context.h"
#include "model/judge/language/Language.h"

int compute_sim_result(int solution_id, int lang, int p_id,
                       const JudgeConfigSnapshot &config,
                       std::shared_ptr<Language> &languageModel,
                       const char *work_dir, int &sim_s_id);

std::string compute_runtime_info(int result, int solution_id,
                                 const JudgeConfigSnapshot &config,
                                 const char *work_dir, bool debug_enabled);

double compute_final_used_time(int result, double timeLimit,
                               const JudgeConfigSnapshot &config,
                               double max_case_time, double usedtime);

#endif
