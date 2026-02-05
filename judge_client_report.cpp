#include "judge_client_report.h"

#include <algorithm>
#include <cstdio>
#include <string>

#include "header/static_var.h"
#include "judge_client_context.h"
#include "judge_client_report_bundle_helpers.h"
#include "judge_client_report_case_helpers.h"
#include "judge_client_report_finalize_helpers.h"
#include "judge_client_report_helpers.h"
#include "judge_client_report_result_helpers.h"
#include "judge_client_report_score_helpers.h"
#include "judge_client_run.h"
#include "judge_client_watch.h"
#include "library/judge_lib.h"
#include "manager/syscall/InitManager.h"

using namespace std;

extern int call_counter[call_array_size];

double clamp_time_limit(double timeLimit) {
    if (timeLimit > 300 * SECOND || timeLimit < ZERO) {
        return 300 * SECOND;
    }
    return timeLimit;
}

int clamp_memory_limit(int memoryLimit) {
    if (memoryLimit > ONE_KILOBYTE || memoryLimit < ONE) {
        return ONE_KILOBYTE;
    }
    return memoryLimit;
}

bool should_continue_cases(int ACflg, const JudgeConfigSnapshot &config) {
    if (!(config.all_test_mode || ACflg == ACCEPT || ACflg == PRESENTATION_ERROR) && ACflg != TIME_LIMIT_EXCEEDED) {
        return false;
    }
    if (ACflg == RUNTIME_ERROR) {
        return false;
    }
    return true;
}

void finalize_result_and_send(int &ACflg, int &finalACflg, int &PEflg, int solution_id, int lang,
                              int p_id, shared_ptr<Language> &languageModel, double timeLimit,
                              int &topmemory, int &pass_point, double &pass_rate, int num_of_test,
                              double &usedtime, double &max_case_time, int &sim, int &sim_s_id,
                              const JudgeConfigSnapshot &config, const char *work_dir,
                              const ResultSender &sender, bool debug_enabled) {
    auto payload = build_result_payload(ACflg, finalACflg, PEflg, solution_id, lang, p_id, config,
                                        languageModel, timeLimit, topmemory, pass_point, pass_rate,
                                        num_of_test, usedtime, max_case_time, sim, sim_s_id,
                                        work_dir, debug_enabled);
    send_result_bundle(payload, sender);
}

void finalize_submission(const string &judgerId, char *work_dir) {
    clean_workdir(work_dir);
    string judger = judgerId;
    removeSubmissionInfo(judger);
}

