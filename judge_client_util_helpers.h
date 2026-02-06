#ifndef JUDGE_CLIENT_UTIL_HELPERS_H
#define JUDGE_CLIENT_UTIL_HELPERS_H

#include <string>

namespace judge_util_helpers {

struct ParsedArgs {
    bool debug = false;
    bool no_record = false;
    bool record_call = false;
    bool admin = false;
    bool no_sim = false;
    bool disable_mysql = false;
    bool read_from_stdin = false;
    bool has_lang_name = false;
    bool has_dir = false;
    bool has_solution_id = false;
    bool has_runner_id = false;
    bool has_judger_id = false;
    std::string lang_name;
    std::string dir;
    int solution_id = 0;
    int runner_id = 0;
    std::string judger_id;
};

struct InitRuntimeConfig {
    bool debug = false;
    bool no_record = false;
    bool record_call = false;
    bool admin = false;
    bool no_sim = false;
    bool disable_mysql = false;
    bool read_from_stdin = false;
    bool has_lang_name = false;
    bool has_dir = false;
    bool has_solution_id = false;
    bool has_runner_id = false;
    bool has_judger_id = false;
    std::string lang_name;
    std::string dir;
    int solution_id = 0;
    int runner_id = 0;
    std::string judger_id;
};

bool parse_new_args(int argc, char **argv, ParsedArgs &parsed);
InitRuntimeConfig build_runtime_from_parsed(const ParsedArgs &parsed);
InitRuntimeConfig build_runtime_from_legacy(int argc, char **argv);
void apply_runtime_to_outputs(const InitRuntimeConfig &runtime,
                              int &solution_id, int &runner_id, std::string &judgerId);
void apply_runtime_to_globals(const InitRuntimeConfig &runtime);

}  // namespace judge_util_helpers

#endif
