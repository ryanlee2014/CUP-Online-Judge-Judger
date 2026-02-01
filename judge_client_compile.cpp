#include "judge_client_compile.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

#include "header/static_var.h"
#include "judge_client_compile_helpers.h"
#include "library/judge_lib.h"
#include "model/base/JSONVectorReader.h"
#include "model/judge/language/Language.h"

using namespace std;
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
    MysqlConfigValues cfg = read_mysql_config(configDIR);
    strcpy(host_name, cfg.hostname.c_str());
    strcpy(user_name, cfg.username.c_str());
    strcpy(password, cfg.password.c_str());
    strcpy(db_name, cfg.db_name.c_str());
    database_port = cfg.port;
    javaTimeBonus = cfg.java_time_bonus;
    java_memory_bonus = cfg.java_memory_bonus;
    strcpy(java_xms, cfg.java_xms.c_str());
    strcpy(java_xmx, cfg.java_xmx.c_str());
    sim_enable = cfg.sim_enable;
    full_diff = cfg.full_diff;
    strcpy(http_username, cfg.judger_name.c_str());
    SHARE_MEMORY_RUN = cfg.shm_run;
    use_max_time = cfg.use_max_time;
    use_ptrace = cfg.use_ptrace;
    ALL_TEST_MODE = cfg.all_test_mode;
    enable_parallel = cfg.enable_parallel;
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
