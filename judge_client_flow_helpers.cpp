#include "judge_client_flow_helpers.h"

#include <filesystem>

#include "judge_client_report.h"
#include "library/judge_lib.h"
#include "manager/syscall/InitManager.h"

using namespace std;

static void load_language_name_cached(const std::string &path) {
    static std::string cached_path;
    static std::filesystem::file_time_type cached_mtime{};
    static bool has_mtime = false;
    std::error_code ec;
    auto mtime = std::filesystem::last_write_time(path, ec);
    if (!ec) {
        if (has_mtime && path == cached_path && mtime == cached_mtime) {
            return;
        }
        cached_mtime = mtime;
        has_mtime = true;
    } else {
        has_mtime = false;
    }
    cached_path = path;
    languageNameReader.loadFile(path);
}

std::string build_run_dir(int runner_id) {
    char work_dir[BUFFER_SIZE];
    snprintf(work_dir, sizeof(work_dir), "%s/run%d/", oj_home, runner_id);
    return string(work_dir);
}

void prepare_work_dir(const JudgeConfigSnapshot &config, char *work_dir) {
    clean_workdir(work_dir);
    if (config.share_memory_run) {
        mk_shm_workdir(work_dir);
    } else {
        make_workdir(work_dir);
    }
}

void prepare_environment(JudgeContext &ctx, int runner_id, FlowState &state) {
    state.global_work_dir = build_run_dir(runner_id);
    snprintf(state.work_dir, sizeof(state.work_dir), "%s", state.global_work_dir.c_str());
    prepare_work_dir(ctx.config, state.work_dir);
    load_language_name_cached(string(oj_home) + "/etc/language.json");
    load_submission(ctx, state.work_dir);
    ctx.time_limit = ctx.language_model->buildTimeLimit(ctx.time_limit, ctx.config.java_time_bonus);
    ctx.memory_limit = ctx.language_model->buildMemoryLimit(ctx.memory_limit, ctx.config.java_memory_bonus);
    ctx.language_model->setExtraPolicy(oj_home, state.work_dir);
    if (ctx.p_id <= TEST_RUN_SUBMIT) {
        ctx.special_judge = NONE_SPECIAL_JUDGE;
    }
    ctx.time_limit = clamp_time_limit(ctx.time_limit);
    ctx.memory_limit = clamp_memory_limit(ctx.memory_limit);
    if (ctx.flags.debug) {
        printf("time: %f mem: %d\n", ctx.time_limit, ctx.memory_limit);
    }
    snprintf(state.fullpath, sizeof(state.fullpath), "%s/data/%d", oj_home, ctx.p_id);
}

void prepare_syscall_template(const JudgeContext &ctx, vector<int> &syscall_template,
                              const int *&syscall_template_ptr) {
    if (!ctx.flags.record_call) {
        InitManager::initSyscallLimits(ctx.lang, syscall_template.data(), ctx.flags.record_call, call_array_size);
        syscall_template_ptr = syscall_template.data();
    }
}
