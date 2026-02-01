#ifndef JUDGE_CLIENT_FLOW_RUNNER_H
#define JUDGE_CLIENT_FLOW_RUNNER_H

#include <utility>
#include <vector>

#include "judge_client_context.h"
#include "judge_client_flow_helpers.h"

void run_cases(int runner_id, int solution_id, JudgeContext &ctx, FlowState &state,
               std::vector<std::pair<std::string, int>> &inFileList,
               const int *syscall_template_ptr);

#endif
