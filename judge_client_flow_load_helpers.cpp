#include "judge_client_flow_load_helpers.h"

#include "judge_client_flow_path_helpers.h"
#include "judge_client_flow_prep_helpers.h"

void load_language_cache() {
    load_language_name_cached(std::string(oj_home) + "/etc/language.json");
}

void load_submission_data(JudgeContext &ctx, FlowState &state) {
    load_submission(ctx, state.work_dir);
    auto fullpath = build_problem_data_path(ctx.p_id);
    snprintf(state.fullpath, sizeof(state.fullpath), "%s", fullpath.c_str());
}
