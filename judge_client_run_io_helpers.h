#ifndef JUDGE_CLIENT_RUN_IO_HELPERS_H
#define JUDGE_CLIENT_RUN_IO_HELPERS_H

#include <string>

namespace judge_run_helpers {

void prepare_io_paths(const char *work_dir, const char *inputFile, const char *userOutputFile,
                      const char *errorOutputFile, std::string &input_path,
                      std::string &output_path, std::string &error_path);

void redirect_stdio(const std::string &input_path, const std::string &output_path, const std::string &error_path);

}  // namespace judge_run_helpers

#endif
