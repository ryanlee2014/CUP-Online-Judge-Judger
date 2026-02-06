#ifndef JUDGE_CLIENT_RUNTIME_BRIDGE_H
#define JUDGE_CLIENT_RUNTIME_BRIDGE_H

#include "judge_client_context.h"
#include "judge_client_util_helpers.h"

void apply_bootstrap_runtime_to_context(JudgeContext &ctx,
                                        const judge_util_helpers::InitRuntimeConfig &runtime);
void apply_bootstrap_globals_from_runtime(const judge_util_helpers::InitRuntimeConfig &runtime);

#endif
