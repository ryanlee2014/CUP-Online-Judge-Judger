#ifndef JUDGE_CLIENT_REPORT_CASE_HELPERS_H
#define JUDGE_CLIENT_REPORT_CASE_HELPERS_H

#include <string>
#include <utility>

#include "judge_client_case_executor.h"
#include "judge_client_context.h"
#include "model/judge/language/Language.h"

struct RunSingleTestcaseRequest {
    int runner_id = 0;
    int solution_id = 0;
    int num_of_test = 0;
    int memory_limit = 0;
    double time_limit = 0;
    char *usercode = nullptr;
    const int *syscall_template = nullptr;
    const std::pair<std::string, int> *infile_pair = nullptr;
};

void run_single_testcase(JudgeContext &ctx, const RunSingleTestcaseRequest &request,
                         JudgePaths &paths, std::string &global_work_dir, CaseExecutionState &state);

#endif
