#include "judge_client_flow_prep_helpers.h"

#include <filesystem>

#include "library/judge_lib.h"

using namespace std;

void load_language_name_cached(const std::string &path) {
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

std::string build_run_dir(int runner_id, const JudgeEnv &env) {
    char work_dir[BUFFER_SIZE];
    snprintf(work_dir, sizeof(work_dir), "%s/run%d/", env.oj_home.c_str(), runner_id);
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
