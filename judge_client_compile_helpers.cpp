#include "judge_client_compile_helpers.h"

#include <filesystem>

CompilerArgsReader &get_compiler_args_reader(const std::string &path) {
    static CompilerArgsReader cached_reader;
    static std::string cached_path;
    static std::filesystem::file_time_type cached_mtime{};
    static bool has_mtime = false;
    std::error_code ec;
    auto mtime = std::filesystem::last_write_time(path, ec);
    bool should_reload = (path != cached_path);
    if (!ec) {
        should_reload = should_reload || !has_mtime || mtime != cached_mtime;
    } else {
        should_reload = should_reload || !has_mtime;
    }
    if (should_reload) {
        cached_reader.loadFile(path);
        cached_path = path;
        if (!ec) {
            cached_mtime = mtime;
            has_mtime = true;
        } else {
            has_mtime = false;
        }
    }
    return cached_reader;
}

MysqlConfigValues read_mysql_config(const std::string &path) {
    MysqlConfigValues values;
    JSONVectorReader configReader(path);
    values.hostname = configReader.GetString("hostname");
    values.username = configReader.GetString("username");
    values.password = configReader.GetString("password");
    values.db_name = configReader.GetString("db_name");
    values.port = configReader.GetInt("port");
    values.java_time_bonus = configReader.GetInt("java_time_bonus");
    values.java_memory_bonus = configReader.GetInt("java_memory_bonus");
    values.java_xms = configReader.GetString("java_xms");
    values.java_xmx = configReader.GetString("java_xmx");
    values.sim_enable = configReader.GetInt("sim_enable");
    values.full_diff = configReader.GetInt("full_diff");
    values.judger_name = configReader.GetString("judger_name");
    values.shm_run = configReader.GetInt("shm_run");
    values.use_max_time = configReader.GetInt("use_max_time");
    values.use_ptrace = configReader.GetInt("use_ptrace");
    values.all_test_mode = configReader.GetInt("all_test_mode");
    values.enable_parallel = configReader.GetInt("enable_parallel");
    return values;
}
