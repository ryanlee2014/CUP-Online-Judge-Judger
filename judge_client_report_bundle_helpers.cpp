#include "judge_client_report_bundle_helpers.h"

void send_compiling_bundle(int solution_id, const ResultSender &sender) {
    bundle.clear();
    bundle.setJudger(http_username);
    bundle.setSolutionID(solution_id);
    bundle.setResult(COMPILING);
    bundle.setFinished(NOT_FINISHED);
    bundle.setUsedTime(ZERO_TIME);
    bundle.setMemoryUse(ZERO_MEMORY);
    bundle.setPassPoint(ZERO_PASSPOINT);
    bundle.setPassRate(ZERO_PASSRATE);
    sender(bundle.toJSONString());
}

void send_running_bundle(int solution_id, int total_point, bool include_total, const ResultSender &sender) {
    bundle.setJudger(http_username);
    bundle.setSolutionID(solution_id);
    bundle.setResult(RUNNING_JUDGING);
    if (include_total) {
        bundle.setTotalPoint(total_point);
    }
    sender(bundle.toJSONString());
}

void send_progress_update(double usedtime, int topmemory, double timeLimit, int memoryLimit,
                          int pass_point, double pass_rate, const ResultSender &sender) {
    update_bundle_progress(usedtime, topmemory, timeLimit, memoryLimit, pass_point, pass_rate);
    sender(bundle.toJSONString());
}
