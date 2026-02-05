#pragma once

#include <string>

namespace judge_path_utils {

std::string join_path(const char *base, const char *name);
void build_parallel_io_names(int file_id, char *input, char *userOutput, char *errorOutput);
void build_parallel_error_name(int file_id, char *errorOutput);

}  // namespace judge_path_utils
