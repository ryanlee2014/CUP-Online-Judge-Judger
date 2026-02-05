#ifndef JUDGE_CLIENT_CONTEXT_HELPERS_H
#define JUDGE_CLIENT_CONTEXT_HELPERS_H

#include "judge_client_compile_helpers.h"
#include "judge_client_context.h"

void apply_mysql_config(const MysqlConfigValues &cfg, JudgeContext &ctx);
JudgeConfigSnapshot capture_config_snapshot();
JudgeRuntimeFlags capture_runtime_flags();
JudgeEnv capture_env();

void load_submission_mysql(JudgeContext &ctx, char *work_dir);
void load_submission_json(JudgeContext &ctx, char *work_dir);

#endif
