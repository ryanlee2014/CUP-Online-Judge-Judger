#ifndef JUDGE_CLIENT_CONTEXT_HELPERS_H
#define JUDGE_CLIENT_CONTEXT_HELPERS_H

#include "judge_client_context.h"

JudgeConfigSnapshot capture_config_snapshot();
JudgeRuntimeFlags capture_runtime_flags();

void load_submission_mysql(JudgeContext &ctx, char *work_dir);
void load_submission_json(JudgeContext &ctx, char *work_dir);

#endif
