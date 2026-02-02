#include "judge_client_run_limit_helpers.h"

#include <sys/resource.h>
#include <unistd.h>

#include "header/static_var.h"
#include "model/judge/language/Language.h"

namespace judge_run_helpers {

void apply_cpu_time_limit(const JudgeConfigSnapshot &config, double time_lmt, double usedtime) {
    struct rlimit lim {};
    if (config.all_test_mode) {
        lim.rlim_cur = static_cast<rlim_t>(time_lmt + 1);
    } else {
        lim.rlim_cur = static_cast<rlim_t>((time_lmt - usedtime / 1000) + 1);
    }
    lim.rlim_max = lim.rlim_cur;
    setrlimit(RLIMIT_CPU, &lim);
    alarm(0);
    alarm(static_cast<unsigned int>(time_lmt * 10));
}

void apply_file_size_limit() {
    struct rlimit lim {};
    lim.rlim_max = static_cast<rlim_t>((STD_F_LIM << 2) + STD_MB);
    lim.rlim_cur = static_cast<rlim_t>(STD_F_LIM << 2);
    setrlimit(RLIMIT_FSIZE, &lim);
}

void apply_stack_limit() {
    struct rlimit lim {};
    lim.rlim_cur = static_cast<rlim_t>(STD_MB << 7);
    lim.rlim_max = static_cast<rlim_t>(STD_MB << 7);
    setrlimit(RLIMIT_STACK, &lim);
}

void apply_memory_limit(Language &languageModel, int mem_lmt, double mem_cur_factor, double mem_max_factor) {
    struct rlimit lim {};
    lim.rlim_cur = static_cast<rlim_t>(STD_MB * mem_lmt * mem_cur_factor);
    lim.rlim_max = static_cast<rlim_t>(STD_MB * mem_lmt * mem_max_factor);
    languageModel.runMemoryLimit(lim);
}

}  // namespace judge_run_helpers
