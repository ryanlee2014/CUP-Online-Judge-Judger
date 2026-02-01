#pragma once

#include <functional>
#include <memory>
#include <string>

#include "header/static_var.h"
#include "judge_client_adapter.h"
#include "model/judge/language/Language.h"
#include "model/submission/SubmissionInfo.h"

struct JudgeConfigSnapshot {
    std::string host_name;
    std::string user_name;
    std::string password;
    std::string db_name;
    std::string http_username;
    int database_port = 0;
    int java_time_bonus = 0;
    int java_memory_bonus = 0;
    int sim_enable = 0;
    int full_diff = 0;
    int share_memory_run = 0;
    int use_max_time = 0;
    int use_ptrace = 0;
    int all_test_mode = 0;
    int enable_parallel = 0;
};

struct JudgeRuntimeFlags {
    int debug = 0;
    char record_call = 0;
    bool admin = false;
    bool no_sim = false;
    bool mysql_mode = true;
    bool read_from_stdin = false;
};

struct JudgeContext {
    std::string judger_id;
    JudgeConfigSnapshot config;
    JudgeRuntimeFlags flags;
    SubmissionInfo submission;
    std::shared_ptr<ISubmissionAdapter> adapter;
    std::shared_ptr<Language> language_model;
    std::function<void(const std::string &)> sender;
    int p_id = 0;
    int lang = 0;
    int special_judge = 0;
    int memory_limit = 0;
    double time_limit = 0.0;
    char user_id[BUFFER_SIZE]{};
    char usercode[CODESIZE]{};
};

void load_config(JudgeContext &ctx);
void load_submission(JudgeContext &ctx, char *work_dir);
