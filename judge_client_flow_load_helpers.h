#ifndef JUDGE_CLIENT_FLOW_LOAD_HELPERS_H
#define JUDGE_CLIENT_FLOW_LOAD_HELPERS_H

#include "judge_client_context.h"
#include "judge_client_flow_helpers.h"

void load_language_cache();
void load_submission_data(JudgeContext &ctx, FlowState &state);

#endif
