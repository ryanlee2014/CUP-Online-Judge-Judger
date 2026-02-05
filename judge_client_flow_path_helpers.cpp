#include "judge_client_flow_path_helpers.h"

#include <string>

std::string build_problem_data_path(int problem_id, const JudgeEnv &env) {
    return env.oj_home + "/data/" + std::to_string(problem_id);
}
