#include "judge_client_flow_path_helpers.h"

#include <string>

#include "header/static_var.h"

std::string build_problem_data_path(int problem_id) {
    return std::string(oj_home) + "/data/" + std::to_string(problem_id);
}
