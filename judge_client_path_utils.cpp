#include "judge_client_path_utils.h"

#include <filesystem>
#include <cstdio>

#include "header/static_var.h"

namespace judge_path_utils {

std::string join_path(const char *base, const char *name) {
    return (std::filesystem::path(base) / name).string();
}

void build_parallel_io_names(int file_id, char *input, char *userOutput, char *errorOutput) {
    snprintf(input, BUFFER_SIZE, "data%d.in", file_id);
    snprintf(userOutput, BUFFER_SIZE, "user%d.out", file_id);
    snprintf(errorOutput, BUFFER_SIZE, "error%d.out", file_id);
}

void build_parallel_error_name(int file_id, char *errorOutput) {
    snprintf(errorOutput, BUFFER_SIZE, "error%d.out", file_id);
}

}  // namespace judge_path_utils
