#include "judge_client_report_payload_helpers.h"

#include "judge_client_report_helpers.h"
#include "library/judge_lib.h"

static bool should_check_sim(const JudgeConfigSnapshot &config,
                             std::shared_ptr<Language> &languageModel) {
    return config.sim_enable && languageModel->enableSim();
}

int compute_sim_result(int solution_id, int lang, int p_id,
                       const JudgeConfigSnapshot &config,
                       std::shared_ptr<Language> &languageModel,
                       const char *work_dir, int &sim_s_id) {
    if (!should_check_sim(config, languageModel)) {
        sim_s_id = 0;
        return 0;
    }
    return get_sim(solution_id, lang, p_id, sim_s_id, work_dir);
}

std::string compute_runtime_info(int result, int solution_id,
                                 const JudgeConfigSnapshot &config,
                                 const char *work_dir, bool debug_enabled) {
    (void)config;
    return build_runtime_info(result, solution_id, work_dir, debug_enabled);
}

double compute_final_used_time(int result, double timeLimit,
                               const JudgeConfigSnapshot &config,
                               double max_case_time, double usedtime) {
    double out = usedtime;
    adjust_usedtime_for_result(result, timeLimit, config.use_max_time, out, max_case_time);
    return out;
}
