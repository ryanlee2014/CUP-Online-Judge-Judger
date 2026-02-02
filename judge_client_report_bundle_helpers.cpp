#include "judge_client_report_bundle_helpers.h"

struct BundleBuilder {
    enum class Kind { Compiling, Running };

    static void set_identity(int solution_id) {
        bundle.setJudger(http_username);
        bundle.setSolutionID(solution_id);
    }

    static void build(Kind kind, int solution_id, int total_point = 0, bool include_total = false) {
        if (kind == Kind::Compiling) {
            bundle.clear();
        }
        set_identity(solution_id);
        if (kind == Kind::Compiling) {
            bundle.setResult(COMPILING);
            bundle.setFinished(NOT_FINISHED);
            bundle.setUsedTime(ZERO_TIME);
            bundle.setMemoryUse(ZERO_MEMORY);
            bundle.setPassPoint(ZERO_PASSPOINT);
            bundle.setPassRate(ZERO_PASSRATE);
        } else {
            bundle.setResult(RUNNING_JUDGING);
            if (include_total) {
                bundle.setTotalPoint(total_point);
            }
        }
    }
};

void send_compiling_bundle(int solution_id, const ResultSender &sender) {
    BundleBuilder::build(BundleBuilder::Kind::Compiling, solution_id);
    sender(bundle.toJSONString());
}

void send_running_bundle(int solution_id, int total_point, bool include_total, const ResultSender &sender) {
    BundleBuilder::build(BundleBuilder::Kind::Running, solution_id, total_point, include_total);
    sender(bundle.toJSONString());
}

void send_progress_update(double usedtime, int topmemory, double timeLimit, int memoryLimit,
                          int pass_point, double pass_rate, const ResultSender &sender) {
    update_bundle_progress(usedtime, topmemory, timeLimit, memoryLimit, pass_point, pass_rate);
    sender(bundle.toJSONString());
}
