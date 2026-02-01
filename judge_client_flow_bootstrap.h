#ifndef JUDGE_CLIENT_FLOW_BOOTSTRAP_H
#define JUDGE_CLIENT_FLOW_BOOTSTRAP_H

#include "judge_client_context.h"
#include "judge_client_flow_helpers.h"

void init_flow(int argc, char **argv, JudgeContext &ctx, FlowState &state,
               int &solution_id, int &runner_id);
void finalize_flow(const JudgeContext &ctx, const FlowState &state);

void init_runtime_flags(int argc, char **argv, JudgeContext &ctx, int &solution_id, int &runner_id);
void init_sender(JudgeContext &ctx, int runner_id, FlowState &state);

#endif
