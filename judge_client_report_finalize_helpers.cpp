#include "judge_client_report_finalize_helpers.h"

#include "judge_client_report_helpers.h"

int resolve_final_result(int ac, int final_ac, int pe, const JudgeConfigSnapshot &config) {
    int result = config.all_test_mode ? final_ac : ac;
    if (result == ACCEPT && pe == PRESENTATION_ERROR) {
        return PRESENTATION_ERROR;
    }
    return result;
}

void apply_runtime_info_and_time(int result, int solution_id, const JudgeConfigSnapshot &config,
                                 const char *work_dir, bool debug_enabled,
                                 double timeLimit, double max_case_time,
                                 double &usedtime, std::string &runtimeInfo) {
    runtimeInfo = build_runtime_info(result, solution_id, work_dir, debug_enabled);
    adjust_usedtime_for_result(result, timeLimit, config.use_max_time, usedtime, max_case_time);
}
