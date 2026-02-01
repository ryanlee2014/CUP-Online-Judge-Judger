#include "judge_client_context.h"

#include <cstring>

#include "header/static_var.h"
#include "judge_client_compile.h"
#include "library/judge_lib.h"

using namespace std;

static JudgeConfigSnapshot capture_config() {
    JudgeConfigSnapshot snapshot;
    snapshot.host_name = host_name;
    snapshot.user_name = user_name;
    snapshot.password = password;
    snapshot.db_name = db_name;
    snapshot.http_username = http_username;
    snapshot.database_port = database_port;
    snapshot.java_time_bonus = javaTimeBonus;
    snapshot.java_memory_bonus = java_memory_bonus;
    snapshot.sim_enable = sim_enable;
    snapshot.full_diff = full_diff;
    snapshot.share_memory_run = SHARE_MEMORY_RUN;
    snapshot.use_max_time = use_max_time;
    snapshot.use_ptrace = use_ptrace;
    snapshot.all_test_mode = ALL_TEST_MODE;
    snapshot.enable_parallel = enable_parallel;
    return snapshot;
}

static JudgeRuntimeFlags capture_flags() {
    JudgeRuntimeFlags flags;
    flags.debug = DEBUG;
    flags.record_call = record_call;
    flags.admin = admin;
    flags.no_sim = no_sim;
    flags.mysql_mode = MYSQL_MODE;
    flags.read_from_stdin = READ_FROM_STDIN;
    return flags;
}

void load_config(JudgeContext &ctx) {
    init_mysql_conf();
    ctx.config = capture_config();
    ctx.flags = capture_flags();
}

static void load_submission_mysql(JudgeContext &ctx, char *work_dir) {
    ctx.adapter = shared_ptr<ISubmissionAdapter>(new MySQLSubmissionAdapterWrapper(getAdapter()));
    ctx.adapter->setPort(database_port)
            .setDBName(db_name)
            .setUserName(user_name)
            .setHostName(host_name)
            .setPassword(password);
    if (!ctx.adapter->start()) {
        cerr << "Failed to create a MYSQL connection." << endl;
        exit(1);
    }
    ctx.adapter->getSolutionInfo(solution_id, ctx.p_id, ctx.user_id, ctx.lang);
    ctx.language_model = shared_ptr<Language>(getLanguageModel(ctx.lang));
    ctx.adapter->getProblemInfo(abs(ctx.p_id), ctx.time_limit, ctx.memory_limit, ctx.special_judge);
    ctx.adapter->getSolution(solution_id, work_dir, ctx.lang, ctx.usercode,
                             ctx.language_model->getFileSuffix().c_str(), ctx.flags.debug);
    ctx.submission.setLanguage(ctx.lang);
    ctx.submission.setSolutionId(solution_id);
    ctx.submission.setTimeLimit(ctx.time_limit);
    ctx.submission.setMemoryLimit(ctx.memory_limit);
    ctx.submission.setProblemId(ctx.p_id);
    ctx.submission.setSpecialJudge(ctx.special_judge);
}

static void load_submission_json(JudgeContext &ctx, char *work_dir) {
    buildSubmissionInfo(ctx.submission, ctx.judger_id);
    getSolutionInfoFromSubmissionInfo(ctx.submission, ctx.p_id, ctx.user_id, ctx.lang);
    ctx.language_model = shared_ptr<Language>(getLanguageModel(ctx.lang));
    getProblemInfoFromSubmissionInfo(ctx.submission, ctx.time_limit, ctx.memory_limit, ctx.special_judge);
    getSolutionFromSubmissionInfo(ctx.submission, ctx.usercode, work_dir);
}

void load_submission(JudgeContext &ctx, char *work_dir) {
    if (ctx.flags.mysql_mode) {
        load_submission_mysql(ctx, work_dir);
    } else {
        load_submission_json(ctx, work_dir);
    }
}
