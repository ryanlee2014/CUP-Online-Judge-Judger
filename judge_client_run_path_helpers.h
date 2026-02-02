#pragma once

#include <string>

namespace judge_run_helpers {

std::string join_path(const char *base, const char *name);
void set_child_work_dir(const char *work_dir);
int read_env_int(const char *name);
void build_parallel_io_names(int file_id, char *input, char *userOutput, char *errorOutput);

}  // namespace judge_run_helpers
