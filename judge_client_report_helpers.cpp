#include "judge_client_report_helpers.h"

#include <algorithm>
#include <filesystem>
#include <iostream>

#include "library/judge_lib.h"

using namespace std;

void send_test_run_bundle(int solution_id, double usedtime, int topmemory, const string &test_run_out,
                          const ResultSender &sender) {
    bundle.clear();
    bundle.setSolutionID(solution_id);
    bundle.setResult(TEST_RUN);
    bundle.setFinished(FINISHED);
    bundle.setUsedTime(usedtime);
    bundle.setMemoryUse(topmemory / ONE_KILOBYTE);
    bundle.setPassPoint(ZERO_PASSPOINT);
    bundle.setPassRate(ZERO_PASSRATE);
    bundle.setTestRunResult(test_run_out);
    sender(bundle.toJSONString());
}

std::string join_report_path(const char *base, const char *name) {
    return (std::filesystem::path(base) / name).string();
}

void update_bundle_progress(double usedtime, int topmemory, double timeLimit, int memoryLimit,
                            int pass_point, double pass_rate) {
    bundle.setUsedTime(std::min(usedtime, timeLimit * 1000));
    bundle.setMemoryUse(std::min(topmemory / ONE_KILOBYTE, memoryLimit * STD_MB / ONE_KILOBYTE));
    bundle.setPassPoint(pass_point);
    bundle.setPassRate(pass_rate);
}

void adjust_usedtime_for_result(int result, double timeLimit, bool use_max_time,
                                double &usedtime, double max_case_time) {
    if (use_max_time) {
        usedtime = max_case_time;
    }
    if (result == TIME_LIMIT_EXCEEDED) {
        usedtime = timeLimit * 1000;
    }
}

std::string build_runtime_info(int result, int solution_id, const char *work_dir, bool debug_enabled) {
    std::string runtimeInfo;
    if (result == RUNTIME_ERROR) {
        runtimeInfo = getRuntimeInfoContents(join_report_path(work_dir, "error.out").c_str());
        if (debug_enabled)
            printf("add RE info of %d..... \n", solution_id);
    }
    if (result == WRONG_ANSWER || result == PRESENTATION_ERROR) {
        runtimeInfo = getRuntimeInfoContents(join_report_path(work_dir, "diff.out").c_str());
        if (debug_enabled)
            printf("add diff info of %d..... \n", solution_id);
    }
    return runtimeInfo;
}

