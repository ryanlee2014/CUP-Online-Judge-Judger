#include "judge_client_context_helpers.h"

#include <cstdlib>
#include <iostream>

#include "header/static_var.h"
#include "library/judge_lib.h"

using namespace std;

namespace {
std::shared_ptr<ISubmissionAdapter> default_adapter_factory() {
    return shared_ptr<ISubmissionAdapter>(new MySQLSubmissionAdapterWrapper(getAdapter()));
}

std::shared_ptr<Language> default_language_factory(int lang) {
    return shared_ptr<Language>(getLanguageModel(lang));
}
} // namespace

void apply_mysql_config(const MysqlConfigValues &cfg, JudgeContext &ctx) {
    ctx.env.host_name = cfg.hostname;
    ctx.env.user_name = cfg.username;
    ctx.env.password = cfg.password;
    ctx.env.db_name = cfg.db_name;
    ctx.env.database_port = cfg.port;
    ctx.env.http_username = cfg.judger_name;

    ctx.config.java_time_bonus = cfg.java_time_bonus;
    ctx.config.java_memory_bonus = cfg.java_memory_bonus;
    ctx.config.sim_enable = cfg.sim_enable;
    ctx.config.full_diff = cfg.full_diff;
    ctx.config.share_memory_run = cfg.shm_run;
    ctx.config.use_max_time = cfg.use_max_time;
    ctx.config.use_ptrace = cfg.use_ptrace;
    ctx.config.all_test_mode = cfg.all_test_mode;
    ctx.config.enable_parallel = cfg.enable_parallel;
    ctx.config.java_xms = cfg.java_xms;
    ctx.config.java_xmx = cfg.java_xmx;

    ctx.flags.use_ptrace = cfg.use_ptrace;
    ctx.flags.all_test_mode = cfg.all_test_mode;
    ctx.flags.enable_parallel = cfg.enable_parallel;
}

void apply_bootstrap_legacy_state(JudgeContext &ctx) {
    JudgeEnv legacy_env = capture_env();
    JudgeRuntimeFlags legacy_flags = capture_runtime_flags();
    ctx.env.oj_home = legacy_env.oj_home;
    ctx.env.http_baseurl = legacy_env.http_baseurl;
    ctx.env.http_password = legacy_env.http_password;
    ctx.flags.debug = legacy_flags.debug;
    ctx.flags.record_call = legacy_flags.record_call;
    ctx.flags.admin = legacy_flags.admin;
    ctx.flags.no_sim = legacy_flags.no_sim;
    ctx.flags.mysql_mode = legacy_flags.mysql_mode;
    ctx.flags.read_from_stdin = legacy_flags.read_from_stdin;
}

JudgeConfigSnapshot capture_config_snapshot() {
    JudgeConfigSnapshot snapshot;
    snapshot.host_name = host_name;
    snapshot.user_name = user_name;
    snapshot.password = password;
    snapshot.db_name = db_name;
    snapshot.http_username = http_username;
    snapshot.java_xms = java_xms;
    snapshot.java_xmx = java_xmx;
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

JudgeRuntimeFlags capture_runtime_flags() {
    JudgeRuntimeFlags flags;
    flags.debug = DEBUG;
    flags.record_call = record_call;
    flags.use_ptrace = use_ptrace;
    flags.all_test_mode = ALL_TEST_MODE;
    flags.enable_parallel = enable_parallel;
    flags.admin = admin;
    flags.no_sim = no_sim;
    flags.mysql_mode = MYSQL_MODE;
    flags.read_from_stdin = READ_FROM_STDIN;
    return flags;
}

JudgeEnv capture_env() {
    JudgeEnv env;
    env.host_name = host_name;
    env.user_name = user_name;
    env.password = password;
    env.db_name = db_name;
    env.oj_home = oj_home;
    env.http_baseurl = http_baseurl;
    env.http_username = http_username;
    env.http_password = http_password;
    env.database_port = database_port;
    return env;
}

void load_submission_mysql(JudgeContext &ctx, char *work_dir) {
    if (!ctx.adapter_factory) {
        ctx.adapter_factory = default_adapter_factory;
    }
    if (!ctx.language_factory) {
        ctx.language_factory = default_language_factory;
    }
    ctx.adapter = ctx.adapter_factory();
    ctx.adapter->setPort(ctx.env.database_port)
            .setDBName(ctx.env.db_name)
            .setUserName(ctx.env.user_name)
            .setHostName(ctx.env.host_name)
            .setPassword(ctx.env.password);
    if (!ctx.adapter->start()) {
        cerr << "Failed to create a MYSQL connection." << endl;
        exit(1);
    }
    ctx.adapter->getSolutionInfo(solution_id, ctx.p_id, ctx.user_id, ctx.lang);
    ctx.language_model = ctx.language_factory(ctx.lang);
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

void load_submission_json(JudgeContext &ctx, char *work_dir) {
    if (!ctx.language_factory) {
        ctx.language_factory = default_language_factory;
    }
    buildSubmissionInfo(ctx.submission, ctx.judger_id);
    getSolutionInfoFromSubmissionInfo(ctx.submission, ctx.p_id, ctx.user_id, ctx.lang);
    ctx.language_model = ctx.language_factory(ctx.lang);
    getProblemInfoFromSubmissionInfo(ctx.submission, ctx.time_limit, ctx.memory_limit, ctx.special_judge);
    getSolutionFromSubmissionInfo(ctx.submission, ctx.usercode, work_dir);
}
