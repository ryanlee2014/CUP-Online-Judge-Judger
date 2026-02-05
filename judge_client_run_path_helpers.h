#pragma once

#include <string>

#include "judge_client_context.h"

namespace judge_run_helpers {

std::string join_path(const char *base, const char *name);
void set_child_work_dir(const char *work_dir);
int read_env_int(const char *name);
void build_parallel_io_names(int file_id, char *input, char *userOutput, char *errorOutput);
JudgePaths build_case_paths(const char *work_dir, const char *inputFile,
                            const char *userOutputFile, const char *errorOutputFile);

}  // namespace judge_run_helpers
