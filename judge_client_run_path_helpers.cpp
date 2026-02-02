#include "judge_client_run_path_helpers.h"

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <unistd.h>

#include "header/static_var.h"

namespace judge_run_helpers {

std::string join_path(const char *base, const char *name) {
    return (std::filesystem::path(base) / name).string();
}

void set_child_work_dir(const char *work_dir) {
    if (chdir(work_dir) != 0 && DEBUG) {
        perror("chdir");
    }
}

int read_env_int(const char *name) {
    const char *val = std::getenv(name);
    if (!val || !*val) {
        return -1;
    }
    char *end = nullptr;
    long parsed = std::strtol(val, &end, 10);
    if (end == val || *end != '\0' || parsed <= 0 || parsed > INT_MAX) {
        return -1;
    }
    return static_cast<int>(parsed);
}

void build_parallel_io_names(int file_id, char *input, char *userOutput, char *errorOutput) {
    snprintf(input, BUFFER_SIZE, "data%d.in", file_id);
    snprintf(userOutput, BUFFER_SIZE, "user%d.out", file_id);
    snprintf(errorOutput, BUFFER_SIZE, "error%d.out", file_id);
}

}  // namespace judge_run_helpers
