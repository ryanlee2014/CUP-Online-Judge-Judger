#include "judge_client_report_result_helpers.h"

#include "judge_client_report_finalize_helpers.h"
#include "judge_client_report_helpers.h"
#include "judge_client_report_score_helpers.h"
#include "library/judge_lib.h"

ResultBundlePayload build_result_payload(int &ACflg, int &finalACflg, int &PEflg,
                                         int solution_id, int lang, int p_id,
                                         const JudgeConfigSnapshot &config,
                                         std::shared_ptr<Language> &languageModel,
                                         double timeLimit, int &topmemory, int &pass_point,
                                         double &pass_rate, int num_of_test, double &usedtime,
                                         double &max_case_time, int &sim, int &sim_s_id,
                                         const char *work_dir, bool debug_enabled) {
    ResultBundlePayload payload;
    payload.result = resolve_final_result(ACflg, finalACflg, PEflg, config);
    if (config.sim_enable && payload.result == ACCEPT && (languageModel->enableSim())) {
        sim = get_sim(solution_id, lang, p_id, sim_s_id, work_dir);
    } else {
        sim = ZERO_SIM;
    }
    payload.sim = sim;
    payload.sim_s_id = sim_s_id;
    payload.topmemory = topmemory;
    payload.pass_point = pass_point;
    payload.pass_rate = calculate_pass_rate(pass_rate, num_of_test);
    apply_runtime_info_and_time(payload.result, solution_id, config, work_dir, debug_enabled,
                                timeLimit, max_case_time, usedtime, payload.runtimeInfo);
    payload.usedtime = usedtime;
    return payload;
}

void send_result_bundle(const ResultBundlePayload &payload, const ResultSender &sender) {
    bundle.setResult(payload.result);
    bundle.setFinished(FINISHED);
    bundle.setRuntimeInfo(payload.runtimeInfo);
    bundle.setUsedTime(payload.usedtime);
    bundle.setMemoryUse(payload.topmemory / ONE_KILOBYTE);
    bundle.setPassPoint(payload.pass_point);
    bundle.setPassRate(payload.pass_rate);
    bundle.setSim(payload.sim);
    bundle.setSimSource(payload.sim_s_id);
    sender(bundle.toJSONString());
}
