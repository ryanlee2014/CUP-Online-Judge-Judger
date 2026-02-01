#include "judge_client_flow_helpers.h"

#include "judge_client_report.h"
#include "judge_client_flow_prepare_helpers.h"
#include "judge_client_flow_syscall_helpers.h"
#include "library/judge_lib.h"
#include "manager/syscall/InitManager.h"

using namespace std;

void prepare_environment(JudgeContext &ctx, int runner_id, FlowState &state) {
    prepare_paths(ctx, runner_id, state);
    prepare_limits_and_policy(ctx, state);
}
