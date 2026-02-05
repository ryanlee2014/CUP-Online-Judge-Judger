#pragma once

struct JudgeConfigSnapshot;
struct JudgeContext;
struct JudgeEnv;

void init_mysql_conf(JudgeContext &ctx);
int compile(int lang, char *work_dir, const JudgeEnv &env,
            const JudgeConfigSnapshot &config, bool debug);
