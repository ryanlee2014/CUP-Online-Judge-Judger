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
#include "judge_client_context.h"
#include "judge_client_context_helpers.h"
#include "library/judge_lib.h"
#include "model/base/JSONVectorReader.h"
#include "model/judge/language/Language.h"

using namespace std;
void init_mysql_conf() {
    JudgeContext ctx;
    ctx.env.oj_home = oj_home;
    ctx.env.http_baseurl = http_baseurl;
    ctx.env.http_password = http_password;
    init_mysql_conf(ctx);

    std::snprintf(host_name, sizeof(host_name), "%s", ctx.env.host_name.c_str());
    std::snprintf(user_name, sizeof(user_name), "%s", ctx.env.user_name.c_str());
    std::snprintf(password, sizeof(password), "%s", ctx.env.password.c_str());
    std::snprintf(db_name, sizeof(db_name), "%s", ctx.env.db_name.c_str());
    database_port = ctx.env.database_port;
    javaTimeBonus = ctx.config.java_time_bonus;
    java_memory_bonus = ctx.config.java_memory_bonus;
    std::snprintf(java_xms, sizeof(java_xms), "%s", ctx.config.java_xms.c_str());
    std::snprintf(java_xmx, sizeof(java_xmx), "%s", ctx.config.java_xmx.c_str());
    sim_enable = ctx.config.sim_enable;
    full_diff = ctx.config.full_diff;
    std::snprintf(http_username, sizeof(http_username), "%s", ctx.env.http_username.c_str());
    SHARE_MEMORY_RUN = ctx.config.share_memory_run;
    use_max_time = ctx.config.use_max_time;
    use_ptrace = ctx.config.use_ptrace;
    ALL_TEST_MODE = ctx.config.all_test_mode;
    enable_parallel = ctx.config.enable_parallel;
}

void init_mysql_conf(JudgeContext &ctx) {
    string config_dir = ctx.env.oj_home + "/etc/config.json";
    MysqlConfigValues cfg = read_mysql_config(config_dir);
    apply_mysql_config(cfg, ctx);
}

int compile(int lang, char *work_dir, const JudgeEnv &env,
            const JudgeConfigSnapshot &config, bool debug) {
    int pid;
    string configJSONDir = env.oj_home;
    configJSONDir += "/etc/compile.json";
    CompilerArgsReader &compilerArgsReader = get_compiler_args_reader(configJSONDir);
    shared_ptr<Language> languageModel(getLanguageModel(lang));
    pid = fork();
    if (pid == CHILD_PROCESS) {
        languageModel->setCompileProcessLimit();
        languageModel->setCompileMount(work_dir);
        languageModel->setCompileExtraConfig();
        setRunUser();
        if (debug)
            cout << "Lang:" << lang << endl;
        auto args = compilerArgsReader.GetArray(to_string(lang));
        const char *java_xms_arg = config.java_xms.empty() ? "-Xms32m" : config.java_xms.c_str();
        const char *java_xmx_arg = config.java_xmx.empty() ? "-Xmx256m" : config.java_xmx.c_str();
        languageModel->compile(args, java_xms_arg, java_xmx_arg);
        if (debug) {
            cout << "Compilation end!\n" << endl;
        }
        exit(0);
    } else {
        int status = 0;
        waitpid(pid, &status, 0);
        status = languageModel->getCompileResult(status);
        if (debug)
            printf("status=%d\n", status);
        execute_cmd("/bin/umount -f bin usr lib lib64 etc/alternatives proc dev 2>&1 >/dev/null");
        execute_cmd("/bin/umount -f %s/* 2>&1 >/dev/null", work_dir);
        umount(work_dir);
        return status;
    }
    return 0;
}
