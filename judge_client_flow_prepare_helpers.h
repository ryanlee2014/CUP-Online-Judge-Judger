#ifndef JUDGE_CLIENT_FLOW_PREPARE_HELPERS_H
#define JUDGE_CLIENT_FLOW_PREPARE_HELPERS_H

#include "judge_client_context.h"
#include "judge_client_flow_helpers.h"

void prepare_paths(JudgeContext &ctx, int runner_id, FlowState &state);
void prepare_limits_and_policy(JudgeContext &ctx, FlowState &state);
void set_work_paths(int runner_id, FlowState &state);

#endif
