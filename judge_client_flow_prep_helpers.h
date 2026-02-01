#ifndef JUDGE_CLIENT_FLOW_PREP_HELPERS_H
#define JUDGE_CLIENT_FLOW_PREP_HELPERS_H

#include <string>

#include "judge_client_context.h"

void load_language_name_cached(const std::string &path);
std::string build_run_dir(int runner_id);
void prepare_work_dir(const JudgeConfigSnapshot &config, char *work_dir);

#endif
