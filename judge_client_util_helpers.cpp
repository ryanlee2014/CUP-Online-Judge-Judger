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

InitRuntimeConfig build_runtime_from_parsed(const ParsedArgs &parsed) {
    InitRuntimeConfig runtime;
    runtime.debug = parsed.debug;
    runtime.no_record = parsed.no_record;
    runtime.record_call = parsed.record_call;
    runtime.admin = parsed.admin;
    runtime.no_sim = parsed.no_sim;
    runtime.disable_mysql = parsed.disable_mysql;
    runtime.read_from_stdin = parsed.read_from_stdin;
    runtime.has_lang_name = parsed.has_lang_name;
    runtime.has_dir = parsed.has_dir;
    runtime.has_solution_id = parsed.has_solution_id;
    runtime.has_runner_id = parsed.has_runner_id;
    runtime.has_judger_id = parsed.has_judger_id;
    runtime.lang_name = parsed.lang_name;
    runtime.dir = parsed.dir;
    runtime.solution_id = parsed.solution_id;
    runtime.runner_id = parsed.runner_id;
    runtime.judger_id = parsed.judger_id;
    return runtime;
}

InitRuntimeConfig build_runtime_from_legacy(int argc, char **argv) {
    InitRuntimeConfig runtime;
    runtime.debug = (argc > 4);
    runtime.record_call = (argc > 5);
    if (argc > 5 && !strcmp(argv[5], "DEBUG")) {
        runtime.no_record = true;
        runtime.record_call = false;
    }
    if (argc > 5) {
        runtime.has_lang_name = true;
        runtime.lang_name = argv[5];
    }
    runtime.has_dir = true;
    runtime.dir = (argc > 3) ? argv[3] : "/home/judge";
    runtime.has_solution_id = true;
    runtime.solution_id = atoi(argv[1]);
    runtime.has_runner_id = true;
    runtime.runner_id = atoi(argv[2]);
    return runtime;
}

void apply_runtime_to_outputs(const InitRuntimeConfig &runtime, int &solution_id, int &runner_id, string &judgerId) {
    if (runtime.has_solution_id) {
        solution_id = runtime.solution_id;
    }
    if (runtime.has_runner_id) {
        runner_id = runtime.runner_id;
    }
    if (runtime.has_judger_id) {
        judgerId = runtime.judger_id;
    }
}

void apply_runtime_to_globals(const InitRuntimeConfig &runtime) {
    // Legacy compatibility only: bootstrap writes CLI parse results into static globals.
    // Subsequent execution should rely on JudgeContext snapshots, not these globals.
    if (runtime.debug) {
        DEBUG = true;
    }
    if (runtime.no_record) {
        NO_RECORD = 1;
    }
    if (runtime.record_call) {
        record_call = 1;
    }
    if (runtime.admin) {
        admin = true;
    }
    if (runtime.no_sim) {
        no_sim = true;
    }
    if (runtime.disable_mysql) {
        MYSQL_MODE = false;
    }
    if (runtime.read_from_stdin) {
        READ_FROM_STDIN = true;
    }
    if (runtime.has_lang_name) {
        strcpy(LANG_NAME, runtime.lang_name.c_str());
    }
    if (runtime.has_dir) {
        strcpy(oj_home, runtime.dir.c_str());
    }
    if (runtime.has_runner_id) {
        judger_number = runtime.runner_id;
    }
}

}  // namespace judge_util_helpers
