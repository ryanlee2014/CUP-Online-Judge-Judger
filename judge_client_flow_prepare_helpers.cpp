#include "judge_client_flow_prepare_helpers.h"

#include <cstdio>

#include "judge_client_flow_prep_helpers.h"
#include "judge_client_report.h"

void prepare_paths(JudgeContext &ctx, int runner_id, FlowState &state) {
    state.global_work_dir = build_run_dir(runner_id);
    snprintf(state.work_dir, sizeof(state.work_dir), "%s", state.global_work_dir.c_str());
    prepare_work_dir(ctx.config, state.work_dir);
    load_language_name_cached(std::string(oj_home) + "/etc/language.json");
    load_submission(ctx, state.work_dir);
    snprintf(state.fullpath, sizeof(state.fullpath), "%s/data/%d", oj_home, ctx.p_id);
}

void prepare_limits_and_policy(JudgeContext &ctx, FlowState &state) {
    ctx.time_limit = ctx.language_model->buildTimeLimit(ctx.time_limit, ctx.config.java_time_bonus);
    ctx.memory_limit = ctx.language_model->buildMemoryLimit(ctx.memory_limit, ctx.config.java_memory_bonus);
    ctx.language_model->setExtraPolicy(oj_home, state.work_dir);
    if (ctx.p_id <= TEST_RUN_SUBMIT) {
        ctx.special_judge = NONE_SPECIAL_JUDGE;
    }
    ctx.time_limit = clamp_time_limit(ctx.time_limit);
    ctx.memory_limit = clamp_memory_limit(ctx.memory_limit);
    if (ctx.flags.debug) {
        printf("time: %f mem: %d\n", ctx.time_limit, ctx.memory_limit);
    }
}
