#include "judge_client_run_io_helpers.h"

#include <cstdio>

#include "judge_client_run_helpers.h"

namespace judge_run_helpers {

void prepare_io_paths(const char *work_dir, const char *inputFile, const char *userOutputFile,
                      const char *errorOutputFile, std::string &input_path,
                      std::string &output_path, std::string &error_path) {
    input_path = join_path(work_dir, inputFile);
    output_path = join_path(work_dir, userOutputFile);
    error_path = join_path(work_dir, errorOutputFile);
}

void redirect_stdio(const std::string &input_path, const std::string &output_path, const std::string &error_path) {
    freopen(input_path.c_str(), "r", stdin);
    freopen(output_path.c_str(), "w", stdout);
    freopen(error_path.c_str(), "a+", stderr);
}

}  // namespace judge_run_helpers
