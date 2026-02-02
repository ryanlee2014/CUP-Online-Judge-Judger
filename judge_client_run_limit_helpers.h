#pragma once

#include "judge_client_context.h"

class Language;

namespace judge_run_helpers {

void apply_cpu_time_limit(const JudgeConfigSnapshot &config, double time_lmt, double usedtime);
void apply_file_size_limit();
void apply_stack_limit();
void apply_memory_limit(Language &languageModel, int mem_lmt, double mem_cur_factor, double mem_max_factor);

}  // namespace judge_run_helpers
