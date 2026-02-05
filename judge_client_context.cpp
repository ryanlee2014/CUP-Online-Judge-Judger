#include "judge_client_context.h"

#include "judge_client_compile_helpers.h"
#include "judge_client_context_helpers.h"

using namespace std;

void load_config(JudgeContext &ctx) {
    string config_path = ctx.env.oj_home + "/etc/config.json";
    MysqlConfigValues cfg = read_mysql_config(config_path);
    apply_mysql_config(cfg, ctx);
}

void load_submission(JudgeContext &ctx, char *work_dir) {
    if (ctx.flags.mysql_mode) {
        load_submission_mysql(ctx, work_dir);
    } else {
        load_submission_json(ctx, work_dir);
    }
}
