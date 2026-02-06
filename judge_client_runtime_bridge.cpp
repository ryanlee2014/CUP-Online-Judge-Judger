#include "judge_client_runtime_bridge.h"

#include <cstring>

#include "header/static_var.h"

void apply_bootstrap_runtime_to_context(JudgeContext &ctx,
                                        const judge_util_helpers::InitRuntimeConfig &runtime) {
    ctx.flags.debug = runtime.debug;
    ctx.flags.record_call = runtime.record_call;
    ctx.flags.admin = runtime.admin;
    ctx.flags.no_sim = runtime.no_sim;
    ctx.flags.mysql_mode = !runtime.disable_mysql;
    ctx.flags.read_from_stdin = runtime.read_from_stdin;
    if (runtime.has_dir) {
        ctx.env.oj_home = runtime.dir;
    }
    // Legacy defaults still come from global storage during bootstrap.
    ctx.env.http_baseurl = http_baseurl;
    ctx.env.http_password = http_password;
}

void apply_bootstrap_globals_from_runtime(const judge_util_helpers::InitRuntimeConfig &runtime) {
    // Legacy compatibility only: bootstrap writes CLI parse results into static globals.
    if (runtime.debug) {
        DEBUG = true;
    }
    if (runtime.no_record) {
        NO_RECORD = 1;
    }
    if (runtime.record_call) {
        record_call = 1;
    }
    if (runtime.admin) {
        admin = true;
    }
    if (runtime.no_sim) {
        no_sim = true;
    }
    if (runtime.disable_mysql) {
        MYSQL_MODE = false;
    }
    if (runtime.read_from_stdin) {
        READ_FROM_STDIN = true;
    }
    if (runtime.has_lang_name) {
        strcpy(LANG_NAME, runtime.lang_name.c_str());
    }
    if (runtime.has_dir) {
        strcpy(oj_home, runtime.dir.c_str());
    }
    if (runtime.has_runner_id) {
        judger_number = runtime.runner_id;
    }
}

judge_util_helpers::InitRuntimeConfig capture_bootstrap_runtime_from_globals() {
    judge_util_helpers::InitRuntimeConfig runtime;
    runtime.debug = (DEBUG != 0);
    runtime.record_call = (record_call != 0);
    runtime.admin = admin;
    runtime.no_sim = no_sim;
    runtime.disable_mysql = !MYSQL_MODE;
    runtime.read_from_stdin = READ_FROM_STDIN;
    runtime.has_dir = true;
    runtime.dir = oj_home;
    runtime.has_runner_id = true;
    runtime.runner_id = judger_number;
    return runtime;
}
