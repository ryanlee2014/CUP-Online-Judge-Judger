#include "judge_client_util_helpers.h"

#include <cstdlib>
#include <cstring>

#include "header/static_var.h"
#include "library/judge_lib.h"

using namespace std;

namespace judge_util_helpers {

bool parse_new_args(int argc, char **argv, ParsedArgs &parsed) {
    bool error = false;
    for (int i = 1; i < argc; ++i) {
        int argType = detectArgType(argv[i]);
        if (argType == _ERROR) {
            error = true;
            break;
        } else if (argType == _DEBUG) {
            parsed.debug = true;
        } else if (argType == _NO_RECORD) {
            parsed.no_record = true;
        } else if (argType == _RECORD_CALL) {
            parsed.record_call = true;
        } else if (argType == _ADMIN) {
            parsed.admin = true;
        } else if (argType == _NO_SIM) {
            parsed.no_sim = true;
        } else if (argType == _NO_MYSQL) {
            parsed.disable_mysql = true;
        } else if (argType == _STDIN) {
            parsed.read_from_stdin = true;
        } else {
            ++i;
            if (i >= argc) {
                error = true;
                break;
            }
            switch (argType) {
                case _LANG_NAME:
                    parsed.has_lang_name = true;
                    parsed.lang_name = argv[i];
                    break;
                case _DIR:
                    parsed.has_dir = true;
                    parsed.dir = argv[i];
                    break;
                case _SOLUTION_ID:
                    parsed.has_solution_id = true;
                    parsed.solution_id = atoi(argv[i]);
                    break;
                case _RUNNER_ID:
                    parsed.has_runner_id = true;
                    parsed.runner_id = atoi(argv[i]);
                    break;
                case _JUDGER_ID:
                    parsed.has_judger_id = true;
                    parsed.judger_id = argv[i];
                    break;
                default:
                    error = true;
                    break;
            }
        }
    }
    if (error) {
        return false;
    }
    return true;
}

void apply_parsed_args(const ParsedArgs &parsed, int &solution_id, int &runner_id, string &judgerId) {
    if (parsed.debug) {
        DEBUG = true;
    }
    if (parsed.no_record) {
        NO_RECORD = 1;
    }
    if (parsed.record_call) {
        record_call = 1;
    }
    if (parsed.admin) {
        admin = true;
    }
    if (parsed.no_sim) {
        no_sim = true;
    }
    if (parsed.disable_mysql) {
        MYSQL_MODE = false;
    }
    if (parsed.read_from_stdin) {
        READ_FROM_STDIN = true;
    }
    if (parsed.has_lang_name) {
        strcpy(LANG_NAME, parsed.lang_name.c_str());
    }
    if (parsed.has_dir) {
        strcpy(oj_home, parsed.dir.c_str());
    }
    if (parsed.has_solution_id) {
        solution_id = parsed.solution_id;
    }
    if (parsed.has_runner_id) {
        judger_number = parsed.runner_id;
        runner_id = parsed.runner_id;
    }
    if (parsed.has_judger_id) {
        judgerId = parsed.judger_id;
    }
}

}  // namespace judge_util_helpers
