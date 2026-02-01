#ifndef JUDGE_CLIENT_COMPILE_HELPERS_H
#define JUDGE_CLIENT_COMPILE_HELPERS_H

#include <string>

#include "model/base/JSONVectorReader.h"

using CompilerArgsReader = JSONVectorReader;

struct MysqlConfigValues {
    std::string hostname;
    std::string username;
    std::string password;
    std::string db_name;
    std::string java_xms;
    std::string java_xmx;
    std::string judger_name;
    int port = 3306;
    int java_time_bonus = 0;
    int java_memory_bonus = 0;
    int sim_enable = 0;
    int full_diff = 0;
    int shm_run = 0;
    int use_max_time = 0;
    int use_ptrace = 0;
    int all_test_mode = 0;
    int enable_parallel = 0;
};

CompilerArgsReader &get_compiler_args_reader(const std::string &path);
MysqlConfigValues read_mysql_config(const std::string &path);

#endif
