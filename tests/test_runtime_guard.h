#pragma once

#include <cstring>
#include <string>

#include "../header/static_var.h"
#include "../judge_client_context.h"
#include "../judge_client_context_helpers.h"

struct RuntimeTestInputs {
    JudgeConfigSnapshot config;
    bool record_syscall = false;
    bool debug_enabled = false;
};

inline JudgeConfigSnapshot make_config_snapshot() {
    JudgeConfigSnapshot cfg;
    cfg.java_time_bonus = javaTimeBonus;
    cfg.java_memory_bonus = java_memory_bonus;
    cfg.sim_enable = sim_enable;
    cfg.share_memory_run = SHARE_MEMORY_RUN;
    cfg.use_max_time = use_max_time;
    cfg.use_ptrace = use_ptrace;
    cfg.all_test_mode = ALL_TEST_MODE;
    cfg.enable_parallel = enable_parallel;
    cfg.java_xms = java_xms;
    cfg.java_xmx = java_xmx;
    return cfg;
}

inline JudgeEnv make_env_with_home(const std::string &home) {
    std::strcpy(oj_home, home.c_str());
    return capture_env();
}

inline RuntimeTestInputs make_runtime_test_inputs() {
    RuntimeTestInputs runtime;
    runtime.config = make_config_snapshot();
    runtime.record_syscall = (record_call != 0);
    runtime.debug_enabled = (DEBUG != 0);
    return runtime;
}

inline RuntimeTestInputs make_runtime_test_inputs(bool debug_enabled, bool use_ptrace_enabled,
                                                  bool all_test_mode_enabled, bool record_syscall_enabled) {
    DEBUG = debug_enabled ? 1 : 0;
    use_ptrace = use_ptrace_enabled ? 1 : 0;
    ALL_TEST_MODE = all_test_mode_enabled ? 1 : 0;
    record_call = record_syscall_enabled ? 1 : 0;
    return make_runtime_test_inputs();
}

struct ScopedGlobalRuntimeGuard {
    int saved_debug = 0;
    int saved_no_record = 0;
    JudgeConfigSnapshot saved_config;
    JudgeRuntimeFlags saved_flags;
    JudgeEnv saved_env;

    ScopedGlobalRuntimeGuard()
        : saved_debug(DEBUG), saved_no_record(NO_RECORD),
          saved_config(capture_config_snapshot()),
          saved_flags(capture_runtime_flags()),
          saved_env(capture_env()) {}

    ~ScopedGlobalRuntimeGuard() {
        DEBUG = saved_debug;
        NO_RECORD = saved_no_record;
        std::strcpy(host_name, saved_config.host_name.c_str());
        std::strcpy(user_name, saved_config.user_name.c_str());
        std::strcpy(password, saved_config.password.c_str());
        std::strcpy(db_name, saved_config.db_name.c_str());
        std::strcpy(http_username, saved_config.http_username.c_str());
        std::strcpy(java_xms, saved_config.java_xms.c_str());
        std::strcpy(java_xmx, saved_config.java_xmx.c_str());
        database_port = saved_config.database_port;
        javaTimeBonus = saved_config.java_time_bonus;
        java_memory_bonus = saved_config.java_memory_bonus;
        sim_enable = saved_config.sim_enable;
        full_diff = saved_config.full_diff;
        SHARE_MEMORY_RUN = saved_config.share_memory_run;
        use_max_time = saved_config.use_max_time;
        use_ptrace = saved_config.use_ptrace;
        ALL_TEST_MODE = saved_config.all_test_mode;
        enable_parallel = saved_config.enable_parallel;

        record_call = saved_flags.record_call;
        admin = saved_flags.admin;
        no_sim = saved_flags.no_sim;
        MYSQL_MODE = saved_flags.mysql_mode;
        READ_FROM_STDIN = saved_flags.read_from_stdin;

        std::strcpy(oj_home, saved_env.oj_home.c_str());
        std::strcpy(http_baseurl, saved_env.http_baseurl.c_str());
        std::strcpy(http_password, saved_env.http_password.c_str());
    }
};
