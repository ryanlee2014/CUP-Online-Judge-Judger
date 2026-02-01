#ifndef JUDGE_CLIENT_REPORT_RESULT_HELPERS_H
#define JUDGE_CLIENT_REPORT_RESULT_HELPERS_H

#include <string>

#include "judge_client_context.h"
#include "judge_client_report_helpers.h"

struct ResultBundlePayload {
    int result = 0;
    int sim = 0;
    int sim_s_id = 0;
    double usedtime = 0;
    int topmemory = 0;
    int pass_point = 0;
    double pass_rate = 0;
    std::string runtimeInfo;
};

ResultBundlePayload build_result_payload(int &ACflg, int &finalACflg, int &PEflg,
                                         int solution_id, int lang, int p_id,
                                         const JudgeConfigSnapshot &config,
                                         std::shared_ptr<Language> &languageModel,
                                         double timeLimit, int &topmemory, int &pass_point,
                                         double &pass_rate, int num_of_test, double &usedtime,
                                         double &max_case_time, int &sim, int &sim_s_id,
                                         const char *work_dir, bool debug_enabled);

void send_result_bundle(const ResultBundlePayload &payload, const ResultSender &sender);

#endif
