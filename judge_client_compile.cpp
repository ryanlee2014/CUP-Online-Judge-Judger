#include "judge_client_compile.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

#include "header/static_var.h"
#include "library/judge_lib.h"
#include "model/base/JSONVectorReader.h"
#include "model/judge/language/Language.h"

using namespace std;
using CompilerArgsReader = JSONVectorReader;
using ConfigReader = JSONVectorReader;

static CompilerArgsReader &get_compiler_args_reader(const std::string &path) {
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

void init_mysql_conf() {
    strcpy(java_xms, "-Xms32m");
    strcpy(java_xmx, "-Xmx256m");
    host_name[0] = 0;
    user_name[0] = 0;
    password[0] = 0;
    db_name[0] = 0;
    database_port = 3306;
    string configDIR = oj_home;
    configDIR += "/etc/config.json";
    ConfigReader configReader(configDIR);
    strcpy(host_name, configReader.GetString("hostname").c_str());
    strcpy(user_name, configReader.GetString("username").c_str());
    strcpy(password, configReader.GetString("password").c_str());
    strcpy(db_name, configReader.GetString("db_name").c_str());
    database_port = configReader.GetInt("port");
    javaTimeBonus = configReader.GetInt("java_time_bonus");
    java_memory_bonus = configReader.GetInt("java_memory_bonus");
    strcpy(java_xms, configReader.GetString("java_xms").c_str());
    strcpy(java_xmx, configReader.GetString("java_xmx").c_str());
    sim_enable = configReader.GetInt("sim_enable");
    full_diff = configReader.GetInt("full_diff");
    strcpy(http_username, configReader.GetString("judger_name").c_str());
    SHARE_MEMORY_RUN = configReader.GetInt("shm_run");
    use_max_time = configReader.GetInt("use_max_time");
    use_ptrace = configReader.GetInt("use_ptrace");
    ALL_TEST_MODE = configReader.GetInt("all_test_mode");
    enable_parallel = configReader.GetInt("enable_parallel");
}

int compile(int lang, char *work_dir) {
    int pid;
    string configJSONDir = oj_home;
    configJSONDir += "/etc/compile.json";
    CompilerArgsReader &compilerArgsReader = get_compiler_args_reader(configJSONDir);
    shared_ptr<Language> languageModel(getLanguageModel(lang));
    pid = fork();
    if (pid == CHILD_PROCESS) {
        languageModel->setCompileProcessLimit();
        languageModel->setCompileMount(work_dir);
        languageModel->setCompileExtraConfig();
        setRunUser();
        if (DEBUG)
            cout << "Lang:" << lang << endl;
        auto args = compilerArgsReader.GetArray(to_string(lang));
        languageModel->compile(args, java_xms, java_xmx);
        if (DEBUG) {
            cout << "Compilation end!\n" << endl;
        }
        exit(0);
    } else {
        int status = 0;
        waitpid(pid, &status, 0);
        status = languageModel->getCompileResult(status);
        if (DEBUG)
            printf("status=%d\n", status);
        execute_cmd("/bin/umount -f bin usr lib lib64 etc/alternatives proc dev 2>&1 >/dev/null");
        execute_cmd("/bin/umount -f %s/* 2>&1 >/dev/null", work_dir);
        umount(work_dir);
        return status;
    }
}
