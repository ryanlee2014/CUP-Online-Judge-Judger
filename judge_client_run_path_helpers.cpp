#include "judge_client_run_path_helpers.h"

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include "judge_client_path_utils.h"

namespace judge_run_helpers {

std::string join_path(const char *base, const char *name) {
    return judge_path_utils::join_path(base, name);
}

void set_child_work_dir(const char *work_dir, bool debug_enabled) {
    if (chdir(work_dir) != 0 && debug_enabled) {
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
    judge_path_utils::build_parallel_io_names(file_id, input, userOutput, errorOutput);
}

JudgePaths build_case_paths(const char *work_dir, const char *inputFile,
                            const char *userOutputFile, const char *errorOutputFile) {
    JudgePaths paths;
    paths.work_dir = work_dir;
    paths.infile = inputFile;
    paths.userfile = userOutputFile;
    paths.errorfile = errorOutputFile;
    return paths;
}

}  // namespace judge_run_helpers
