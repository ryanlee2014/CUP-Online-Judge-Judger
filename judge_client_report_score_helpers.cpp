#include "judge_client_report_score_helpers.h"

#include <algorithm>

#include "judge_client_report_bundle_helpers.h"

using namespace std;

double calculate_pass_rate(double pass_rate, int num_of_test) {
    if (num_of_test <= 0) {
        return ZERO_PASSRATE;
    }
    return pass_rate / num_of_test;
}

void apply_parallel_result(const JudgeSeriesResult &r, int num_of_test, double timeLimit, int memoryLimit,
                           int &finalACflg, int &ACflg, int &topmemory, double &usedtime,
                           double &max_case_time, int &pass_point, double &pass_rate,
                           const ResultSender &sender) {
    update_bundle_progress(r.usedTime, r.topMemory, timeLimit, memoryLimit, r.pass_point,
                           double(r.pass_point) / max(num_of_test, 1));
    sender(bundle.toJSONString());
    finalACflg = ACflg = r.ACflg;
    topmemory = r.topMemory;
    usedtime = max_case_time = min(r.usedTime, timeLimit * 1000);
    pass_point = pass_rate = r.pass_point;
}
