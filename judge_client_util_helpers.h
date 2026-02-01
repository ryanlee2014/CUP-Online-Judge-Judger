#ifndef JUDGE_CLIENT_UTIL_HELPERS_H
#define JUDGE_CLIENT_UTIL_HELPERS_H

#include <string>

namespace judge_util_helpers {

bool parse_new_args(int argc, char **argv, int &solution_id, int &runner_id, std::string &judgerId);

}  // namespace judge_util_helpers

#endif
