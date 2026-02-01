#include "judge_client_context.h"

#include "judge_client_context_helpers.h"
#include "judge_client_compile.h"

using namespace std;

void load_config(JudgeContext &ctx) {
    init_mysql_conf();
    ctx.config = capture_config_snapshot();
    ctx.flags = capture_runtime_flags();
}

void load_submission(JudgeContext &ctx, char *work_dir) {
    if (ctx.flags.mysql_mode) {
        load_submission_mysql(ctx, work_dir);
    } else {
        load_submission_json(ctx, work_dir);
    }
}
